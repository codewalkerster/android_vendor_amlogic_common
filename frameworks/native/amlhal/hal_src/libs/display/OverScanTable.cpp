/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "CPQTable"

#include <math.h>

#include "CPQTable.h"

CPQTable::CPQTable()
{
    //CPQTable_GammaConvert_Init(256);
}

CPQTable::~CPQTable()
{

}

//==========================Gamma mode ceconvert start======================================
int CPQTable::CPQTable_GammaConvert_Init(int NodeNum)
{
    int a, i, j_min, j_max;
    size_t j;
    double para;

    int n = NodeNum;

    m_upper.resize(2);
    m_lower.resize(2);

    for (j = 0; j < m_upper.size(); j++) {
        m_upper[j].resize(n);
    }

    for (j = 0; j < m_lower.size(); j++) {
        m_lower[j].resize(n);
    }

    xData.resize(n);
    yData.resize(n);
    m_a.resize(n);
    m_b.resize(n);
    m_c.resize(n);

    for (i = 0 ; i < n; ++j) {
        xData[i] = ((double)i)/((double)(n-1));
    }

    //setting upper/lower
    for (i = 1; i < n-1; i++) {
        m_lower[1][i] = 1.0/3.0*(xData[i]-xData[i-1]);
        m_upper[0][i] = 2.0/3.0*(xData[i+1]-xData[i-1]);
        m_upper[1][i] = 1.0/3.0*(xData[i+1]-xData[i]);
    }
    m_upper[0][0]   = 2.0;
    m_upper[1][0]   = 0.0;
    m_upper[0][n-1] = 2.0;
    m_lower[1][n-1] = 0.0;

    for (i = 0; i < n; i++) {
        assert(m_upper[0][i]!=0.0);
        m_lower[0][i] = 1.0/m_upper[0][i];
        j_min = std::max(0,i-1);
        j_max = std::min(n-1,i+1);
        for (a = j_min; a <= j_max; j++) {
            if (i > a)
                m_lower[1][i] *= m_lower[0][i];
            else if(i < a)
                m_upper[1][i] *= m_lower[0][i];
            else
                m_upper[0][i] *= m_lower[0][i];
        }
        m_upper[0][i] = 1.0;              // prevents rounding errors
    }

    // Gauss LR-Decomposition
    for (int k = 0; k < n; k++) {
        int i_max = std::min(n-1,k+1);
        assert(m_upper[0][k] != 0.0);
        para = -m_lower[1][i_max]/m_upper[0][k];
        m_lower[1][i_max] = -para;        // assembly part of L
        m_upper[0][i_max] = m_upper[0][i_max] + para*m_upper[1][k];
    }
    return 0;

}

int CPQTable::CPQTable_BaseGammaConvert(tcon_gamma_table_t *gamma_value, double basePower , double targetPower)
{
    std::vector<unsigned short> returnCurve;
    int j;
    int ret =-1;
    int n = xData.size();

    if (n < 2) {
        LOGE("%s, gammaNumNode Too little to convert >>>>return!!!<<<<\n", __FUNCTION__);
        return -1;
    }

    for (j = 0 ; j < n; ++j) {
        yData[j] = (double)gamma_value->data[j];
    }

    if (yData.size() != xData.size()) {
        LOGE("%s, gamma SIZE not match can not convert yData size =%d xData.size()=%d\n", __FUNCTION__,yData.size(),xData.size());
        return -1;
    }

    if (basePower == targetPower) {
        ret = GammaConvert_set_points(yData, false);
    } else {
        ret  =GammaConvert_set_points(yData, true);
    }

    if (ret < 0) {
        LOGE("%s, GammaConvert_set_points error!!!\n", __FUNCTION__);
    } else {
        for (j = 0 ; j < n; ++j) {
            returnCurve.push_back(GammaConvert_Get_points(pow(xData[j] , targetPower/basePower )));
            gamma_value->data[j]=returnCurve[j];

        }
    }
    return ret;
}

int CPQTable::GammaConvert_set_points(const std::vector<double>& y, bool cubic_spline)
{
    int i;
    double sum_x,sum_y;
    int n=y.size();
    std::vector<double> rhs(n);
    std::vector<double> x(n);

    if (y.size() != xData.size()) {
        LOGE("%s, gamma SIZE not match can not convert y size =%d xData.size()=%d\n", __FUNCTION__,y.size(),xData.size());
        return -1;
    }

    if (cubic_spline == true) {
        for (i = 1; i < n-1; i++) {
            rhs[i] = (y[i+1] - y[i]) / (xData[i+1] - xData[i]) - (y[i] - y[i-1]) / (xData[i] - xData[i-1]);
        }
        rhs[0]   = 0.0;
        rhs[n-1] = 0.0;

        for (i = 0; i < n; i++) {
            sum_x = 0;
            if (i == 0) {
                x[i] = rhs[i]*m_lower[0][i];
            } else {
                sum_x +=m_lower[1][i]*x[i-1];
                x[i] = (rhs[i]*m_lower[0][i]) - sum_x;
            }
        }

        for (i = n-1; i >= 0; i--) {
            sum_y = 0;
            if (i == n-1) {
                m_b[i] = x[i]/m_upper[0][i];
            } else {
                sum_y += m_upper[1][i] * m_b[i+1];
                m_b[i]=( x[i] - sum_y ) / m_upper[0][i];
            }
        }

        for (i = 0; i < n-1; i++) {
            m_a[i] = 1.0/3.0*(m_b[i+1]-m_b[i])/(xData[i+1]-xData[i]);
            m_c[i] = (y[i+1]-y[i])/(y[i+1]-y[i])- 1.0/3.0*(2.0*m_b[i]+m_b[i+1])*(xData[i+1]-xData[i]);
        }
        double h = xData[n-1] - xData[n-2];
        m_a[n-1] = 0.0;
        m_c[n-1] = 3.0*m_a[n-2]*h*h+2.0*m_b[n-2]*h+m_c[n-2];
    }else {
        for (int i = 0; i < n-1; i++) {
            m_a[i] = 0.0;
            m_b[i] = 0.0;
            m_c[i] = (yData[i+1]-yData[i])/(xData[i+1]-xData[i]);
        }
    }
    return 0;
}

double CPQTable::GammaConvert_Get_points(double x)
{
    // find the closest point m_x[idx] < x, idx=0 even if x<m_x[0]
    std::vector<double>::const_iterator it;
    it = std::lower_bound(xData.begin(),xData.end(),x);
    int idx = std::max( int(it-xData.begin())-1, 0);
    int n = xData.size();

    double h = x-xData[idx];
    double interpol;

    if (x < xData[0]) {
         // extrapolation to the left
         interpol = (m_b[0]*h + m_c[0])*h + xData[0];
     } else if(x > xData[n-1]) {
         // extrapolation to the right
         interpol = (m_b[n-1]*h + m_c[n-1])*h + yData[n-1];
     } else {
         // interpolation
        interpol = ((m_a[idx] * h + m_b[idx]) * h + m_c[idx]) * h + yData[idx];
     }
    return interpol;
}
//=========================Gamma mode ceconvert end  =======================================


//=========================whiteBalan ceconvert start=======================================
interpolation_info_t* CPQTable::nat_cubic_spline(int num_points, interpolation_info_t* output_fun) {
    float x_delta[num_points-1]; /* x differences */
    float y_delta[num_points-1]; /* y differences */
    float A[num_points][NUMBER_POINTS]; /* matrix for solving for c */
    float h[num_points]; /* vector for solving for c */
    /* Goal: create interpolating functions of the form:
       S_j(x) = a_j(x) + b_j(x - x_j) + c_j(x - x_j)^2 + d_j(x - x_j)^3
       where 0 < j < num_points
    */

    /* First, want to solve: Ac = h where A is a matrix and c and h are
       vectors
    */
    int i; /* loop index */
    /* If there aren't enough points to interpolate, bail */
    if (num_points < 3) {
        return NULL;
    }

    /* Assign parameters to our S struct */
    output_fun->num_points = num_points;

    /* Build x diff and y diff */
    for (i = 1; i < num_points; i++) {
        x_delta[i-1] = output_fun->x[i] - output_fun->x[i-1];
        y_delta[i-1] = output_fun->y[i] - output_fun->y[i-1];
    }

    /* Build "a" vector (just y) */
    for (i = 0; i < num_points; i++) {
        output_fun->a[i] = output_fun->y[i];
    }

    /* Build A matrix */
    build_A_matrix(x_delta, num_points, A);
    /* Build h vector */
    build_h_vector(h, x_delta, output_fun->a, num_points);
    /* Solve matrix equation for c vector (Ac = h) */
    solve_matrix(output_fun->c, h, num_points, A);
    /* Build b vector */
    build_b_vector(output_fun->b, x_delta, y_delta, output_fun->c, num_points);
    /* Build d vector */
    build_d_vector(output_fun->d, x_delta, output_fun->c, num_points);
    /* Return S struct containing all the coeffs and init vals */
    return output_fun;
}

void CPQTable::build_A_matrix(float *x_delta, int num_points,
        float A[][NUMBER_POINTS]) {
    int i;

    /* Set top and bottom corners */
    A[0][0] = 1;
    A[num_points-1][num_points-1] = 1;

    /* Fill in the matrix by natural cubic spline algorithm */
    for (i = 1; i < num_points-1; i++) {
        A[i][i-1] = x_delta[i-1];
        A[i][i]   = 2*(x_delta[i-1]+x_delta[i]);
        A[i][i+1] = x_delta[i];
    }
}

float* CPQTable::build_h_vector(float *h, float *x_delta, float *a, int num_points) {
    int i;

    /* Set top and bottom */
    h[0] = 0.;
    h[num_points-1] = 0.;

    /* Fill in the vector by natural cubic spline algorithm */
    for (i = 1; i < num_points-1; i++) {
        h[i] = 3.*((a[i+1]-a[i]) / x_delta[i] - (a[i]-a[i-1]) / x_delta[i-1]);
    }

    return h;
}

float* CPQTable::solve_matrix(float *x, float *h, int num_points,
        float A[][NUMBER_POINTS]) {
    /* Solves tridiagonal matrix equation Ax = h
       for tridiagonal matrix A using Thomas' algorithm.
       This requires the matrix to be diagonally dominant or symmetric
       positive definite. This should always be the case for our spline.
    */

    float a[num_points]; /* values to the left of diagonal of A */
    float b[num_points]; /* values on the diagonal of A */
    float c[num_points]; /* values to the right of diagonal of A */
    int i; /* loop index */
    float w[num_points]; /* used as a temp variable */

    /* Set end points */
    b[0] = 1.;
    c[0] = 0.;

    a[num_points-1] = 0.;
    b[num_points-1] = 1.;

    /* Build a, b, c */
    for (i = 1; i < num_points-1; i++) {
        a[i] = A[i][i-1];
        b[i] = A[i][i];
        c[i] = A[i][i+1];
    }

   /* Apply Thomas' algorithm */
   for (i = 1; i < num_points; i++) {
       w[i] = a[i] / b[i-1];
       b[i] = b[i] - w[i]*c[i-1];
       h[i] = h[i] - w[i]*h[i-1];

   }

   /* Back substitute x */
   x[num_points-1] = h[num_points-1] / b[num_points-1];

   for (i = num_points-2; i >= 0; i--) {
       x[i] = (h[i] - c[i] * x[i+1]) / b[i];
   }

   return x;
}

float* CPQTable::build_b_vector(float *b, float *x_delta, float *y_delta, float *c,
                                        int num_points) {
    int i; /* loop index */

    /* Build b by natural cubic spline */
    for (i = 0; i < num_points-1; i++) {
        b[i] = y_delta[i] / x_delta[i] - x_delta[i] / 3 * (2 * c[i] + c[i+1]);
    }

    return b;
}

float* CPQTable::build_d_vector(float *d, float *x_delta, float *c, int num_points) {

    int i; /* loop index */

    /* Build d by natural cubic spline */
    for (i = 0; i < num_points-1; i++) {
        d[i] = (c[i+1] - c[i]) / (3 * x_delta[i]);
    }

    return d;
}

int CPQTable::evaluate(interpolation_info_t *function, float val, float *result) {
    /* Use the interpolation to evaluate a val, answer stored in
       result
    */

    int i; /* loop index */

    /* if the val is less than the smallest value, outside of range,
       bail.
    */
    if (val < function->x[0]) {
        /* -1 represents too small of a val ... not sure how errno is used
           in arduino so I'm using return values to set error types.
        */
        return -1;
    }

    /* If the val is greater than the largest value, outside of range,
       bail.
    */
    else if (val > function->x[function->num_points-1]) {
        /* Set error val */
        return -2;
    }

    for (i = 0; i < function->num_points-1; i++) {
        /* If val equals an element in my x array, just return the
           corresponding y val
        */
        if (almost_equals(val, function->x[i])) {
            *result = function->y[i];
            /* return val of 0 means no errors */
            return 0;
        }

        /* Check the next element also, since I use the range next */
        else if (almost_equals(val, function->x[i+1])) {
            *result = function->y[i+1];
            return 0;
        }
        else if (val > function->x[i] && val < function->x[i+1]) {
            /* If the val falls between 2 initial x values, find the value the
               interpolation gives.
            */
            *result = spline_func(function, val, i);
            return 0;
        }
    }

    /* Something has gone horribly wrong */
    return -3;
}

float CPQTable::spline_func(interpolation_info_t *function, float val, int i) {
    /* This function selects the appropriate spline function and evaluates
       that function at val.
    */

    /* Each p is a different term in the polynomial (number after p is the
       power)
    */
    float p0;
    float p1;
    float p2;
    float p3;

    /* It's useful to copy and paste the functional form of our spline from
       above:

       S_i(x) = a_i(x) + b_i(x - x_i) + c_i(x - x_i)^2 + d_i(x - x_i)^3
       where 0 < i < num_points.

       We take the ith element from the a, b, c, d, and x arrays for the
       {letter}_i terms. The x term is val in this function. S_i(x) is the
       returned value from the interpolation.
    */

    p0 = function->a[i];
    p1 = function->b[i] * (val - function->x[i]);
    p2 = function->c[i] * (val - function->x[i])*(val - function->x[i]);
    p3 = function->d[i] * (val -
            function->x[i]) * (val - function->x[i]) * (val - function->x[i]);
    return p0 + p1 + p2 + p3;
}

bool CPQTable::almost_equals(float a, float b) {
    /* Comparing floats for exact equality is shady, they won't ever be
       exactly the same, so use this function to test if they're close enough.
    */

    float c = a - b; /* c is the difference */

    /* Make sure c is positive */
    if (c < 0) {
        c = -1*c;
    }

    /* If the difference is sufficiently small, return true */
    if (c < 0.00000001) {
        return true;
    }
    /* Otherwise, return false */
    else {
        return false;
    }
}
//===========================whiteBalan ceconvert end===================================

CPQTable *CPQTable::mInstance = NULL;
CPQTable *CPQTable::GetInstance()
{
    if (NULL == mInstance) {
        mInstance = new CPQTable();
    }
    return mInstance;
}