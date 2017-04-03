/*============================================================================
* File    : integer_division_uint16.c
* Purpose : calculate coefficients for integer division of uint16 values
------------------------------------------------------------------------------
* Version : 1
* Date    : 15th June 2009
* Author  : Alan Bowens
------------------------------------------------------------------------------
* Based on the article "Division of integers by constants" by Nigel Jones,
* http://www.embeddedgurus.net/stack-overflow/2009/06/division-of-integers-by-constants.html
* This article is also cross posted at his website: www.rmbconsulting.us.
*
* The above article was in turn based upon a paper by Doug Jones available 
* at http://www.cs.uiowa.edu/~jones/bcd/divide.html
*
* The latest version of this file may be downloaded from my blog 
* at http://codereview.blogspot.com/.
*
* This code is supplied as is. It comes with no warranty and no guarantee. Use 
* it at your own risk. You are free to use it and modify it in any way you see 
* fit, provided that this notice is included in its entirety.
============================================================================*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* standard integer sizes */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

/* function prototypes */
void gen_recipe_1( uint32_t one_over_K, uint16_t *M, uint16_t *S );
void gen_recipe_2( uint32_t one_over_K, uint16_t *M, uint16_t *S );
uint16_t run_recipe_1( uint16_t A, uint16_t M, uint16_t S );
uint16_t run_recipe_2( uint16_t A, uint16_t M, uint16_t S );
void test_recipe( double K, uint16_t M, uint16_t S, uint16_t (*recipe)( uint16_t A, uint16_t M, uint16_t S ) );
uint16_t calc_S( uint32_t *one_over_K );

/* main */
int main(int argc, char* argv[])
{
   double K;
   uint16_t M;
   uint16_t S;
   uint32_t one_over_K;

   /* check command line arguments */
   if( argc != 2 )
   {
      printf( "usage: integer_division_uint16 constant\n" );
      exit( 1 );
   }

   /* get K from the command line */
   sscanf( argv[1], "%lf", &K );

   /* calculate one over K */
   one_over_K = (uint32_t)( pow( 2.0, 32.0 ) * ( 1.0 / K ) );

   /*
   * calculate coefficients via recipes 1 and 2, and test the resultant
   * equations
   */
   printf( "------------------\n" );
   printf( "Integer division by %lf\n", K );

   printf( "------------------\n" );
   printf( "recipe 1:\n" );
   gen_recipe_1( one_over_K, &M, &S );
   printf( "Q = (((uint32_t)A * (uint32_t)0x%04Xu) >> 16u) >> %uu;\n", M, S );
   test_recipe( K, M, S, run_recipe_1 );

   printf( "------------------\n" );
   printf( "incremented recipe 1:\n" );
   printf( "Q = (((uint32_t)A * (uint32_t)0x%04Xu) >> 16u) >> %uu;\n", ( M + 1 ), S );
   test_recipe( K, (uint16_t)( M + 1 ), S, run_recipe_1 );

   printf( "------------------\n" );
   printf( "recipe 2:\n" );
   gen_recipe_2( one_over_K, &M, &S );
   printf( "Q = (((((uint32_t)A * (uint32_t)0x%04Xu) >> 16u) + A) >> 1) >> %uu;\n", M, S );
   test_recipe( K, M, S, run_recipe_2 );

   printf( "------------------\n" );
   printf( "incremented recipe 2:\n" );
   printf( "Q = (((((uint32_t)A * (uint32_t)0x%04Xu) >> 16u) + A) >> 1) >> %uu;\n", ( M + 1 ), S );
   test_recipe( K, (uint16_t)( M + 1 ), S, run_recipe_2 );

   return 0;
}

/* calculate M and S via recipe 1 */
void gen_recipe_1( uint32_t one_over_K, uint16_t *M, uint16_t *S )
{
   uint32_t most_significant_17_bits;

   /* calculate S */
   *S = calc_S( &one_over_K );

   /*
   * Take the most significant 17 bits and add 1 and then truncate to
   * 16 bits. This effectively rounds the result.
   */
   most_significant_17_bits = ( ( one_over_K & 0xFFFF8000u ) >> 15u );
   most_significant_17_bits++;
   *M = (uint16_t)( most_significant_17_bits >> 1u );
}

/* calculate M and S via recipe 2 */
void gen_recipe_2( uint32_t one_over_K, uint16_t *M, uint16_t *S )
{
   uint32_t most_significant_18_bits;

   /* calculate S */
   *S = calc_S( &one_over_K );

   /*
   * Take the most significant 18 bits and add 1 and then truncate to
   * 17 bits. This effectively rounds the result.
   */
   most_significant_18_bits = ( ( one_over_K & 0xFFFFC000u ) >> 14u );
   most_significant_18_bits++;
   *M = (uint16_t)( most_significant_18_bits >> 1u );
}

/* calculate S, given 1 over K */
uint16_t calc_S( uint32_t *one_over_K )
{
   uint16_t rtnval = 0u;

   /*
   * Take all the bits to the right of the binary point, and left shift
   * them until the bit to the right of the binary point is 1. Record the
   * required number of shifts S.
   */
   while( 0 == ( *one_over_K & 0x80000000 ) )
   {
      *one_over_K <<= 1u;
      rtnval++;
   }

   return rtnval;
}

/* test a given recipe for accuracy over 0..65535 */
void test_recipe( double K, uint16_t M, uint16_t S, uint16_t (*recipe)( uint16_t A, uint16_t M, uint16_t S ) )
{
   uint16_t Q;
   uint16_t error;
   uint16_t max_error = 0u;
   uint16_t num_errors = 0u;
   uint16_t expected_result;
   uint32_t A;

   for( A = 0u; A < 65536; A++ )
   {
      /* calculate the result using floating point division */
      expected_result = (uint16_t)( (double) A / K );

      /* calculate the result using integer division */
      Q = recipe( (uint16_t) A, M, S );

      /* calculate the error */
      error = (uint16_t) abs( expected_result - Q );

      /* record all errors */
      if( error != 0 )
      {
         num_errors++;

         if( error > max_error )
         {
            max_error = error;
         }
      }
   }

   /* display the number and range of errors */
   printf( "num errors = %u\n", num_errors );
   printf( "max error = %u\n", max_error );
}

/* perform integer division using recipe 1 */
uint16_t run_recipe_1( uint16_t A, uint16_t M, uint16_t S )
{
   return (uint16_t)( (((uint32_t)A * (uint32_t)M) >> 16) >> S );
}

/* perform integer division using recipe 2 */
uint16_t run_recipe_2( uint16_t A, uint16_t M, uint16_t S )
{
   return (uint16_t)( (((((uint32_t)A * (uint32_t)M) >> 16) + A) >> 1) >> S );
}

