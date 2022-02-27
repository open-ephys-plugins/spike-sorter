/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "PCAJob.h"

/*
  An implementation of SVD from Numerical Recipes in C and Mike Erhdmann's lectures
*/

#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : - fabs(a))

static double maxarg1, maxarg2;
#define FMAX(a,b) (maxarg1 = (a),maxarg2 = (b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))

static int iminarg1, iminarg2;
#define IMIN(a,b) (iminarg1 = (a),iminarg2 = (b),(iminarg1 < (iminarg2) ? (iminarg1) : iminarg2))

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)


PCAjob::PCAjob(SorterSpikeArray& _spikes, float* _pc1, float* _pc2,
                std::atomic<float>& pc1Min,  std::atomic<float>& pc2Min,  std::atomic<float>&pc1Max,  std::atomic<float>& pc2Max, std::atomic<bool>& _reportDone) : spikes(_spikes),
pc1min(pc1Min), pc2min(pc2Min), pc1max(pc1Max), pc2max(pc2Max), reportDone(_reportDone)
{
	SorterSpikePtr spike = spikes[0];
    cov = nullptr;
    pc1 = _pc1;
    pc2 = _pc2;

    dim = spike->getChannel()->getNumChannels()*spike->getChannel()->getTotalSamples();

};

PCAjob::~PCAjob()
{

}

// calculates sqrt( a^2 + b^2 ) with decent precision
float PCAjob::pythag(float a, float b)
{
    float absa,absb;

    absa = fabs(a);
    absb = fabs(b);

    if (absa > absb)
        return (absa * sqrt(1.0 + SQR(absb/absa)));
    else
        return (absb == 0.0 ? 0.0 : absb * sqrt(1.0 + SQR(absa / absb)));
}

/*
  Modified from Numerical Recipes in C
  Given a matrix a[nRows][nCols], svdcmp() computes its singular value
  decomposition, A = U * W * Vt.  A is replaced by U when svdcmp
  returns.  The diagonal matrix W is output as a vector w[nCols].
  V (not V transpose) is output as the matrix V[nCols][nCols].
*/
int PCAjob::svdcmp(float** a, int nRows, int nCols, float* w, float** v)
{

    int flag, i, its, j, jj, k, l = 0, nm = 0;
    float anorm, c, f, g, h, s, scale, x, y, z, *rv1;

    rv1 = new float[nCols];
    if (rv1 == NULL)
    {
        printf("svdcmp(): Unable to allocate vector\n");
        return (-1);
    }

    g = scale = anorm = 0.0;
    for (i = 0; i < nCols; i++)
    {
        l = i+1;
        rv1[i] = scale*g;
        g = s = scale = 0.0;
        if (i < nRows)
        {
            for (k = i; k < nRows; k++)
            {
                //std::cout << k << " " << i << std::endl;
                scale += fabs(a[k][i]);
            }

            if (scale)
            {
                for (k = i; k < nRows; k++)
                {
                    a[k][i] /= scale;
                    s += a[k][i] * a[k][i];
                }
                f = a[i][i];
                g = -SIGN(sqrt(s),f);
                h = f * g - s;
                a[i][i] = f - g;

                for (j = l; j < nCols; j++)
                {
                    for (s = 0.0, k = i; k < nRows; k++) s += a[k][i] * a[k][j];
                    f = s / h;
                    for (k = i; k < nRows; k++) a[k][j] += f * a[k][i];
                }

                for (k = i; k < nRows; k++)
                    a[k][i] *= scale;
            } // end if (scale)
        } // end if (i < nRows)
        w[i] = scale * g;
        g = s = scale = 0.0;
        if (i < nRows && i != nCols-1)
        {
            for (k = l; k < nCols; k++) scale += fabs(a[i][k]);
            if (scale)
            {
                for (k = l; k < nCols; k++)
                {
                    a[i][k] /= scale;
                    s += a[i][k] * a[i][k];
                }
                f = a[i][l];
                g = - SIGN(sqrt(s),f);
                h = f * g - s;
                a[i][l] = f - g;
                for (k=l; k<nCols; k++) rv1[k] = a[i][k] / h;
                for (j=l; j<nRows; j++)
                {
                    for (s=0.0,k=l; k<nCols; k++) s += a[j][k] * a[i][k];
                    for (k=l; k<nCols; k++) a[j][k] += s * rv1[k];
                }
                for (k=l; k<nCols; k++) a[i][k] *= scale;
            }
        }
        anorm = FMAX(anorm, (fabs(w[i]) + fabs(rv1[i])));


    }

    for (i=nCols-1; i>=0; i--)
    {
        if (i < nCols-1)
        {
            if (g)
            {
                for (j=l; j<nCols; j++)
                    v[j][i] = (a[i][j] / a[i][l]) / g;
                for (j=l; j<nCols; j++)
                {
                    for (s=0.0,k=l; k<nCols; k++) s += a[i][k] * v[k][j];
                    for (k=l; k<nCols; k++) v[k][j] += s * v[k][i];
                }
            }
            for (j=l; j<nCols; j++) v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }

    for (i=IMIN(nRows,nCols) - 1; i >= 0; i--)
    {
        l = i + 1;
        g = w[i];
        for (j=l; j<nCols; j++) a[i][j] = 0.0;
        if (g)
        {
            g = 1.0 / g;
            for (j=l; j<nCols; j++)
            {
                for (s=0.0,k=l; k<nRows; k++) s += a[k][i] * a[k][j];
                f = (s / a[i][i]) * g;
                for (k=i; k<nRows; k++) a[k][j] += f * a[k][i];
            }
            for (j=i; j<nRows; j++) a[j][i] *= g;
        }
        else
            for (j=i; j<nRows; j++) a[j][i] = 0.0;
        ++a[i][i];
    }

    for (k=nCols-1; k>=0; k--)
    {
        for (its=0; its<30; its++)
        {
            flag = 1;
            for (l=k; l>=0; l--)
            {
                nm = l-1;
                if ((fabs(rv1[l]) + anorm) == anorm)
                {
                    flag =  0;
                    break;
                }
                if ((fabs(w[nm]) + anorm) == anorm) break;
            }
            if (flag)
            {
                c = 0.0;
                s = 1.0;
                for (i=l; i<=k; i++)
                {
                    f = s * rv1[i];
                    rv1[i] = c * rv1[i];
                    if ((fabs(f) + anorm) == anorm) break;
                    g = w[i];
                    h = pythag(f,g);
                    w[i] = h;
                    h = 1.0 / h;
                    c = g * h;
                    s = -f * h;
                    for (j=0; j<nRows; j++)
                    {
                        y = a[j][nm];
                        z = a[j][i];
                        a[j][nm] = y * c + z * s;
                        a[j][i] = z * c - y * s;
                    }
                }
            }
            z = w[k];
            if (l == k)
            {
                if (z < 0.0)
                {
                    w[k] = -z;
                    for (j=0; j<nCols; j++) v[j][k] = -v[j][k];
                }
                break;
            }
            //if(its == 29) printf("no convergence in 30 svdcmp iterations\n");
            x = w[l];
            nm = k-1;
            y = w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = pythag(f,1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g,f))) - h)) / x;
            c = s = 1.0;
            for (j=l; j<=nm; j++)
            {
                i = j+1;
                g = rv1[i];
                y = w[i];
                h = s * g;
                g = c * g;
                z = pythag(f,h);
                rv1[j] = z;
                c = f/z;
                s = h/z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y *= c;
                for (jj=0; jj<nCols; jj++)
                {
                    x = v[jj][j];
                    z = v[jj][i];
                    v[jj][j] = x * c + z * s;
                    v[jj][i] = z * c - x * s;
                }
                z = pythag(f,h);
                w[j] = z;
                if (z)
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = c * g + s * y;
                x = c * y - s * g;
                for (jj=0; jj < nRows; jj++)
                {
                    y = a[jj][j];
                    z = a[jj][i];
                    a[jj][j] = y * c + z * s;
                    a[jj][i] = z * c - y * s;
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = x;
        }
    }

    delete[] rv1;

    return (0);
}


void PCAjob::computeCov()
{
    // allocate and zero
    cov = new float*[dim];
    float* mean  = new float[dim];
    for (int k = 0; k < dim; k++)
    {
        cov[k] = new float[dim];
        for (int j=0; j<dim; j++)
        {
            cov[k][j] = 0;
        }
    }
    // compute mean

    for (int j=0; j<dim; j++)
    {
        mean[j] = 0;
        for (int i=0; i<spikes.size(); i++)
        {
            SorterSpikePtr spike = spikes[i];
            float v = spike->spikeDataIndexToMicrovolts(j);
            mean[j] += v / dim;
        }
    }
    // aggregate


    for (int i=0; i<dim; i++)
    {
        for (int j=i; j<dim; j++)
        {
            // cov[i][j] = sum_k[ (X(i,:)) * (Xj-mue(j) ]
            float sum = 0 ;
            for (int k=0; k<spikes.size(); k++)
            {

                SorterSpikePtr spike = spikes[k];
                float vi = spike->spikeDataIndexToMicrovolts(i);
                float vj = spike->spikeDataIndexToMicrovolts(j);
                sum += (vi-mean[i]) * (vj-mean[j]);
            }
            cov[i][j] = sum / (dim-1);
            cov[j][i] = sum / (dim-1);
        }
    }
    delete[] mean;

}

std::vector<int> sort_indexes(std::vector<float> v)
{
    // initialize original index locations
    std::vector<int> idx(v.size());

    for (int i = 0; i != idx.size(); ++i)
    {
        idx[i] = i;
    }

    //sort indexes based on comparing values in v
    sort(
        idx.begin(),
        idx.end()//,
        //[&v](size_t i1, size_t i2)
        //{
        //	return v[i1] > v[i2];
        //}
    );

    return idx;
}

void PCAjob::computeSVD()
{


    float** eigvec, *sigvalues;
    sigvalues = new float[dim];

    eigvec = new float*[dim];
    for (int k = 0; k < dim; k++)
    {
        eigvec[k] = new float[dim];
        for (int j=0; j<dim; j++)
        {
            eigvec[k][j] = 0;
        }
    }

    svdcmp(cov, dim, dim, sigvalues, eigvec);

    std::vector<float> sig;
    sig.resize(dim);
    for (int k = 0; k < dim; k++)
        sig[k] = sigvalues[k];

    std::vector<int> sortind = sort_indexes(sig);

    for (int k = 0; k < dim; k++)
    {
        pc1[k] = eigvec[k][sortind[0]];
        pc2[k] = eigvec[k][sortind[1]];
    }
    // project samples to find the display range
    float min1 = 1e10, min2 = 1e10, max1 = -1e10, max2 = -1e10;

    for (int j = 0; j < spikes.size(); j++)
    {
        float sum1 = 0, sum2=0;
        for (int k = 0; k < dim; k++)
        {
            SorterSpikePtr spike = spikes[j];
            sum1 += spike->spikeDataIndexToMicrovolts(k) * pc1[k];
            sum2 += spike->spikeDataIndexToMicrovolts(k) * pc2[k];
        }
        if (sum1 < min1)
            min1 = sum1;
        if (sum2 < min2)
            min2 = sum2;
        if (sum1 > max1)
            max1 = sum1;
        if (sum2 > max2)
            max2 = sum2;
    }


    pc1min = min1 - 1.5 * (max1-min1);
    pc2min = min2 - 1.5 * (max2-min2);
    pc1max = max1 + 1.5 * (max1-min1);
    pc2max = max2 + 1.5 * (max2-min2);

    // clear memory
    for (int k = 0; k < dim; k++)
    {
        delete[] eigvec[k];
    }
    delete[] eigvec;
    delete[] sigvalues;

    // delete covariances
    for (int k = 0; k < dim; k++)
        delete[] cov[k];

    delete[] cov;
    cov = nullptr;

}


/**************************/
