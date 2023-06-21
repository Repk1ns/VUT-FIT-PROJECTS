/**
 * @file BatchMandelCalculator.cc
 * @author Vojtěch Mimochodek <xmimoc01@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 14.11.2021
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <stdexcept>

#include "BatchMandelCalculator.h"

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
	data = (int *)(_mm_malloc(height * width * sizeof(int), 64));
	pomZReal = (float *)(_mm_malloc(64 * sizeof(float), 64));
	pomZImag = (float *)(_mm_malloc(64 * sizeof(float), 64));
}

BatchMandelCalculator::~BatchMandelCalculator() {
	_mm_free(data);
	_mm_free(pomZReal);
	_mm_free(pomZImag);
	data = NULL;
	pomZReal = NULL;
	pomZImag = NULL;
}


int * BatchMandelCalculator::calculateMandelbrot () {
	int *pdata = data;
	float *ppomZReal = pomZReal;
	float *ppomZImag = pomZImag;

	int part = 64;
	int tmp = 0;
	int index;

	for (int i = 0; i < height; i++)
	{
		float y = y_start + i * dy;

		for (int m = 0; m < width/part; m++) {

			index = m * part;

			pdata = (data + i * width) + m * part;

			#pragma omp simd reduction(+:index) simdlen(64) aligned(ppomZReal: 64) aligned(ppomZImag: 64) aligned(pdata: 64)
			for(int l = 0; l < part; l++) {
				ppomZReal[l] = x_start + index * dx;
				ppomZImag[l] = y;
				pdata[l] = limit;
				index++;
			}
		
			for (int k = 0; k < limit; ++k)
			{
				tmp = 0;
				index = m * part;

				#pragma omp simd reduction(+:tmp) reduction(+:index) simdlen(64) aligned(ppomZReal: 64) aligned(ppomZImag: 64) aligned(pdata: 64)
				for (int j = 0; j < part; j++)
				{
					float x = x_start + index * dx;

					float i2 = ppomZImag[j] * ppomZImag[j];
					float r2 = ppomZReal[j] * ppomZReal[j];

					if((r2 + i2 > 4.0f) && pdata[j] == limit) {
						(pdata[j] = k);
						tmp++;
					}
				
					ppomZImag[j] = 2.0f * ppomZReal[j] * ppomZImag[j] + y;
					ppomZReal[j] = (r2 - i2 + x);
					index++;
				}
				if(tmp == part) break;
			}
		}
	}
	return data;
}