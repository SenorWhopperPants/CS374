/* integral.h declares integral, a function to calculate
 *  definite integrals using the trapezoidal rule.
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 * Edited by Josh Bussis: changed integrateTrap() to accept parameters so that it can be 
 *                        parallelized
 * @date  10/21/2019
 * @where Calvin University
 * @why   Project 05
 * @class CS 374
 */

long double integrateTrap(double xLo, double xHi,
                          unsigned long long numTrapezoids,
                          unsigned long long start, unsigned long long stop, int id);

