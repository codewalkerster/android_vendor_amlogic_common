/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef _CPQTABLE_H
#define _CPQTABLE_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "PQType.h"
#include "CPQLog.h"

#ifndef NUMBER_POINTS
#define NUMBER_POINTS 40
#endif

typedef struct interpolation_info_s {
    float a[NUMBER_POINTS];
    float b[NUMBER_POINTS];
    float c[NUMBER_POINTS];
    float d[NUMBER_POINTS];
    float *x;
    float *y;
    int num_points;
}interpolation_info_t;


class CPQTable       {
public:
    CPQTable();
    ~CPQTable();
    static CPQTable *GetInstance();
    //gamma Convert
    int CPQTable_GammaConvert_Init(int NodeNum);
    int CPQTable_BaseGammaConvert(tcon_gamma_table_t *gamma_value, double basePower , double targetPower);
    //whiteBalanceGamma
    interpolation_info_t* nat_cubic_spline(int num_points, interpolation_info_t* output);
    int evaluate(interpolation_info_t *function, float val, float *result);

private:

    static CPQTable *mInstance;
    //gamma Convert
    int GammaConvert_set_points(const std::vector<double>& y, bool cubic_spline);
    double GammaConvert_Get_points(double x);
    //whiteBalanceGamma
    void build_A_matrix(float *x_delta, int num_points, float A[][NUMBER_POINTS]);
    float* build_h_vector(float *h, float *x_delta, float *a, int num_points);
    float* solve_matrix(float *x, float *h, int num_points, float A[][NUMBER_POINTS]);
    float* build_b_vector(float *b, float *x_delta, float *y_delta, float *c, int num_points);
    float* build_d_vector(float *d, float *x_delta, float *c, int num_points);
    float spline_func(interpolation_info_t *function, float val, int i);
    bool almost_equals(float a, float b);

    std::vector<double> xData,yData;            // x,y coordinates of points
    std::vector< std::vector<double> > m_upper;  // upper band
    std::vector< std::vector<double> > m_lower;  // lower band
    std::vector<double> m_a,m_b,m_c;        // spline coefficients


};

#endif

