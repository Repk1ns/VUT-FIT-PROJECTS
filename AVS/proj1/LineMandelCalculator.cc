/**
 * @file LineMandelCalculator.cc
 * @author Vojtěch Mimochodek <xmimoc01@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 14.11.2021
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
	data = (int *)(_mm_malloc(height * width * sizeof(int), 64));
	pomZReal = (float *)(_mm_malloc(width * sizeof(float), 64));
	pomZImag = (float *)(_mm_malloc(width * sizeof(float), 64));
}

LineMandelCalculator::~LineMandelCalculator() {
	_mm_free(data);
	_mm_free(pomZReal);
	_mm_free(pomZImag);
	data = NULL;
	pomZReal = NULL;
	pomZImag = NULL;
}


int * LineMandelCalculator::calculateMandelbrot () {

	int *pdata = data;
	float *ppomZReal = pomZReal;
	float *ppomZImag = pomZImag;
	int tmp = 0;

	for (int i = 0; i < height; i++)
	{
		float y = y_start + i * dy;
		pdata = data + i * width;

		#pragma omp simd simdlen(64) aligned(pdata: 64) aligned(ppomZReal: 64) aligned(ppomZImag: 64)
		for(int l = 0; l < width; l++) {
			ppomZReal[l] = x_start + l * dx;
			ppomZImag[l] = y;
			pdata[l] = limit;
		}
		tmp = 0;
		
		for (int k = 0; k < limit; ++k)
		{
			
			#pragma omp simd reduction(+:tmp) simdlen(64) aligned(pdata: 64) aligned(ppomZReal: 64) aligned(ppomZImag: 64)
			for (int j = 0; j < width; j++)
			{
				float x = x_start + j * dx;

				float r2 = ppomZReal[j] * ppomZReal[j];
				float i2 = ppomZImag[j] * ppomZImag[j];

				if((r2 + i2 > 4.0f) && pdata[j] == limit) {
					(pdata[j] = k);
					tmp++;
				}
				
				ppomZImag[j] = 2.0f * ppomZReal[j] * ppomZImag[j] + y;
				ppomZReal[j] = (r2 - i2 + x);
			}
			if(tmp == width) break;
		}
	}
	return data;
}
