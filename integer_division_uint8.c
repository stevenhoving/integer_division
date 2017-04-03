/*============================================================================
* File    : integer_division_uint8.c
* Purpose : calculate coefficients for integer division of uint8 values
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
void gen_recipe_1_uint8( uint32_t one_over_K, uint8_t *M, uint8_t *S );
void gen_recipe_2_uint8( uint32_t one_over_K, uint8_t *M, uint8_t *S );
uint8_t run_recipe_1_uint8( uint8_t A, uint8_t M, uint8_t S );
uint8_t run_recipe_2_uint8( uint8_t A, uint8_t M, uint8_t S );
void test_recipe_uint8( double K, uint8_t M, uint8_t S, uint8_t (*recipe)( uint8_t A, uint8_t M, uint8_t S ) );
uint16_t calc_S( uint32_t *one_over_K );

/* main */
int main(int argc, char* argv[])
{
   double K;
   uint8_t M8;
   uint8_t S8;
   uint32_t one_over_K;

   /* check command line arguments */
   if( argc != 2 )
   {
      printf( "usage: integer_division_uint8 constant\n" );
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
   gen_recipe_1_uint8( one_over_K, &M8, &S8 );
   printf( "Q = (((uint16_t)A * (uint16_t)0x%02Xu) >> 8u) >> %uu;\n", M8, S8 );
   test_recipe_uint8( K, M8, S8, run_recipe_1_uint8 );

   printf( "------------------\n" );
   printf( "incremented recipe 1:\n" );
   printf( "Q = (((uint16_t)A * (uint16_t)0x%02Xu) >> 8u) >> %uu;\n", ( M8 + 1 ), S8 );
   test_recipe_uint8( K, (uint16_t)( M8 + 1 ), S8, run_recipe_1_uint8 );

   printf( "------------------\n" );
   printf( "recipe 2:\n" );
   gen_recipe_2_uint8( one_over_K, &M8, &S8 );
   printf( "Q = (((((uint16_t)A * (uint16_t)0x%02Xu) >> 8u) + A) >> 1u) >> %uu;\n", M8, S8 );
   test_recipe_uint8( K, M8, S8, run_recipe_2_uint8 );

   printf( "------------------\n" );
   printf( "incremented recipe 2:\n" );
   printf( "Q = (((((uint16_t)A * (uint16_t)0x%02Xu) >> 8u) + A) >> 1u) >> %uu;\n", ( M8 + 1 ), S8 );
   test_recipe_uint8( K, (uint16_t)( M8 + 1 ), S8, run_recipe_2_uint8 );

   return 0;
}

/* calculate M and S via recipe 1 */
void gen_recipe_1_uint8( uint32_t one_over_K, uint8_t *M, uint8_t *S )
{
   uint32_t most_significant_9_bits;

   /* calculate S */
   *S = calc_S( &one_over_K );

   /*
   * Take the most significant 9 bits and add 1 and then truncate to
   * 8 bits. This effectively rounds the result.
   */
   most_significant_9_bits = ( ( one_over_K & 0xFF800000u ) >> 23u );
   most_significant_9_bits++;
   *M = (uint8_t)( most_significant_9_bits >> 1u );
}

/* calculate M and S via recipe 2 */
void gen_recipe_2_uint8( uint32_t one_over_K, uint8_t *M, uint8_t *S )
{
   uint32_t most_significant_10_bits;

   /* calculate S */
   *S = calc_S( &one_over_K );

   /*
   * Take the most significant 10 bits and add 1 and then truncate to
   * 9 bits. This effectively rounds the result.
   */
   most_significant_10_bits = ( ( one_over_K & 0xFFC00000u ) >> 22u );
   most_significant_10_bits++;
   *M = (uint8_t)( most_significant_10_bits >> 1u );
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

/* test a given recipe for accuracy over 0..255 */
void test_recipe_uint8( double K, uint8_t M, uint8_t S, uint8_t (*recipe)( uint8_t A, uint8_t M, uint8_t S ) )
{
   uint16_t A;
   uint16_t Q;
   uint16_t error;
   uint16_t max_error = 0u;
   uint16_t num_errors = 0u;
   uint16_t expected_result;

   for( A = 0u; A < 256; A++ )
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
uint8_t run_recipe_1_uint8( uint8_t A, uint8_t M, uint8_t S )
{
   return (uint8_t)( (((uint16_t)A * (uint16_t)M) >> 8u) >> S );
}

/* perform integer division using recipe 2 */
uint8_t run_recipe_2_uint8( uint8_t A, uint8_t M, uint8_t S )
{
   return (uint8_t)( (((((uint16_t)A * (uint16_t)M) >> 8u) + A) >> 1u) >> S );
}

