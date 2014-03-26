/*********************************************************************
 * This file manages times.  It includes routines that will figure out
 * the difference between two times, as well as a method which will sum time, 
 * and a method which will convert time from units into ms.
 * Created by Walter Schilling, Winter 2013-2014.
 */
#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <time.h>
#include <stdint.h>
 
 /*******************************************************************************
 * This method will determine the difference between two timspec structures.
 * @param struct timespec *result - This is a pointer to a structure that will be 
 *                                  populated with a timing result.
 * @param struct timespec *x - This is a pointer to a structure that will be populated 
 *                             with a timing result.  This is the starting time.
 * @param struct timespec *y - This is a pointer to a structure that will be populated 
 *                             with a timing result.  This is the ending time.
 * @return The return will be 0 if the difference is positive.  Non zero if negative.
 ******************************************************************************/
int  timeval_subtract (  struct timespec  *result, struct timespec *x,struct timespec *y);
	 
/*******************************************************************************
 * This method will determine the sum two timspec structures.
 * @param struct timespec *result - This is a pointer to a structure that will be 
 *                                  populated with a timing result.
 * @param struct timespec *x - This is a pointer to a structure that will be populated 
 *                             with a timing result.  This is the starting time.
 * @param struct timespec *y - This is a pointer to a structure that will be populated 
 *                             with a timing result.  This is the ending time.
 * @return The return will be 0 if the sum is positive.  Non zero if negative.
 ******************************************************************************/
int  timeval_add (  struct timespec  *result, struct timespec *x,struct timespec *y);
	 
/*******************************************************************************
 * This method will calculate the number of ms present in the given time structure.
 * @param struct timespec *result - This is a pointer to a structure that will be 
 *                                  populated with a timing result.
 * @return The return will be the number of ms represented by the time structure.
 ******************************************************************************/
uint32_t timespectoms(struct timespec *structure);

#endif
