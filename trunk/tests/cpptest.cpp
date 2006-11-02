#include "Simmath.h"
#include "SimTKcommon.h"
#include "SimTKcommon/internal/common.h"
#include "simmatrix/internal/BigMatrix.h"
#include "Optimizer.h"
#include "ObjectiveFunction.h"

#include <iostream>
using std::cout;
using std::endl;


#define PROBLEM_DIMENSION 2

class objfunc : public SimTK::ObjectiveFunction {

   double getValue(  SimTK::Vector &coefficients ) {
      double x, y;

      x = coefficients[0];
      y = coefficients[1];

      return( 0.5*(3*x*x+4*x*y+6*y*y) - 2*x + 8*y); 

   }

   void getGradient(  SimTK::Vector &coefficients, SimTK::Vector &gradient ){
      double x, y;

      x = coefficients[0]; 
      y = coefficients[1];  

      gradient[0] = 3*x + 2*y -2;
      gradient[1] = 2*x + 6*y +8; 

   }

};

/* adapted from itkLBFGSOptimizerTest.cxx */
main() {

    double params[10];
    int i;

    objfunc of;
    SimTK::Vector results(2);

    cout << "cpptest " << endl;
    try {
    SimTK::Optimizer opt( PROBLEM_DIMENSION ); 

    params[0] = 0;
    opt.setOptimizerParameters( TRACE, params );

    params[0] = 100;
    opt.setOptimizerParameters( MAX_FUNCTION_EVALUATIONS, params );

    params[0] = .0001;
    opt.setOptimizerParameters( GRADIENT_CONVERGENCE_TOLERANCE, params );

    params[0] = 1.0;
    opt.setOptimizerParameters( DEFAULT_STEP_LENGTH, params );

    params[0] = 0.9;
    opt.setOptimizerParameters( LINE_SEARCH_ACCURACY, params );

    opt.setObjectiveFunction( &of );

    results[0] =  100;
    results[1] = -100;
    
    opt.optimize( results );

    }

    catch (SimTK::Exception::Base exp) {
        cout << "Caught exception :" << exp.getMessage() << endl;
    }

    for( i=0; i<PROBLEM_DIMENSION; i++ ) {
       printf(" results[%d] = %f \n",i,results[i]); 
    }
}