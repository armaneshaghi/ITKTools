#include "itkCommandLineArgumentParser.h"
#include "CommandLineArgumentHelper.h"

/*#include "itkImage.h"
#include "itkUnaryFunctorImageFilter.h"
#include <map>
#include <utility>
#include <vector>
#include <itksys/SystemTools.hxx>

#include "itkUnaryFunctors.h"
#include "UnaryImageOperatorHelper.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
*/
#include "UnaryImageOperatorMainHelper.h"

/** Functions to do the work: this splits things up
 * in different compilation units.
 */
extern int UnaryImageOperatorScalar( const std::string & inputFileName,
  const std::string & outputFileName, const std::string & ops,
  const std::string & argument, const unsigned int inputDimension,
  const std::string & ComponentTypeIn, const std::string & ComponentTypeOut );

//-------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
	/** Check arguments for help. */
	if ( argc < 5 || argc > 11 )
	{
		PrintHelp();
		return 1;
	}

	/** Create a command line argument parser. */
	itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
	parser->SetCommandLineArguments( argc, argv );

	/** Get arguments. */
	std::string inputFileName = "";
	bool retin = parser->GetCommandLineArgument( "-in", inputFileName );
  std::string	outputFileName = "";
	bool retout = parser->GetCommandLineArgument( "-out", outputFileName );
  std::string ops = "PLUS";
  bool retops = parser->GetCommandLineArgument( "-ops", ops );
  std::string argument = "1";
  bool retarg = parser->GetCommandLineArgument( "-arg", argument );
  
  /** Check if the required arguments are given. */
  if ( !retin )
	{
		std::cerr << "ERROR: You should specify \"-in\"." << std::endl;
		return 1;
	}
  if ( !retops )
	{
		std::cerr << "ERROR: You should specify \"-ops\"." << std::endl;
		return 1;
	}

  /** Create outputFileName. */
  if ( outputFileName == "" )
  {
    CreateOutputFileName( inputFileName, outputFileName, ops, argument );
  }

  /** Determine image properties. */
  std::string inputComponentType = "short";
  std::string	inputPixelType = "";
  unsigned int inputDimension = 2;
  unsigned int numberOfComponents = 1;
  int retgip = GetImageProperties( inputFileName, inputPixelType,
    inputComponentType, inputDimension, numberOfComponents);
  if ( retgip ) return retgip;

  /** Get the output component type. */
  std::string ComponentTypeOut = inputComponentType;
	bool retopt = parser->GetCommandLineArgument( "-pto", ComponentTypeOut );
  ReplaceUnderscoreWithSpace( ComponentTypeOut );

  /** The input is only templated over int and double. */
  bool inputIsInteger;
  InputIsInteger( inputComponentType, inputIsInteger );
  std::string ComponentTypeIn = "double";
  if ( inputIsInteger ) ComponentTypeIn = "int";
  
  /** Get the correct form of ops. For some operators
   * there are integer and double versions, in which case
   * ops is concatenated with INT or DOUBLE. For example
   * for the ABS functor this is true. For these functors
   * the decision is made based on the inputComponentType
   * and the argument type. If both are of integer type then
   * INT is used, otherwise DOUBLE.
   */
  bool argumentIsInteger;
  ArgumentIsInteger( argument, argumentIsInteger );
  /** Append ops and at the same time check if ops is a valid
   * functor.
   */
  std::string opsOld = ops;
  int retCO  = CheckOps( ops, inputIsInteger & argumentIsInteger );
  if ( retCO ) return retCO;

  /** For certain ops an argument is mandatory. */
  bool operatorNeedsArgument = OperatorNeedsArgument( opsOld );
  if ( operatorNeedsArgument && !retarg )
  {
    std::cerr << "ERROR: operator " << opsOld << " needs an argument." << std::endl;
    std::cerr << "Specify the argument with \"-arg\"." << std::endl;
    return 1;
  }
  if ( !operatorNeedsArgument && retarg )
  {
    std::cerr << "WARNING: operator " << opsOld << " does not need an argument." << std::endl;
    std::cerr << "The argument (" << argument << ") is ignored." << std::endl;
  }

  /** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
   * Now that we have all arguments available, run!
   */
  try
	{
    /** Scalar support. */
    if ( inputPixelType == "scalar" && numberOfComponents == 1 )
    {
      const int ret_value = UnaryImageOperatorScalar( inputFileName,
        outputFileName, ops, argument, inputDimension,
        ComponentTypeIn, ComponentTypeOut );
      if ( ret_value != 0 ) return ret_value;
    }
    /** Other image types are not supported. */
    else
    {
      std::cerr << "The pixel type is " << inputPixelType
        << "and the number of components is "
        << numberOfComponents << std::endl;
      std::cerr << "ERROR: Vector images are not supported!" << std::endl;
      return 1;
    }
  } // end try
  catch( itk::ExceptionObject &e )
	{
		std::cerr << "Caught ITK exception: " << e << std::endl;
		return 1;
	}
		
	/** End program. */
	return 0;

} // end main

