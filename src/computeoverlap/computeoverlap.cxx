#include "itkCommandLineArgumentParser.h"
#include "CommandLineArgumentHelper.h"

#include "itkImageFileReader.h"
#include "itkImage.h"
#include "itkAndImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkThresholdLabelerImageFilter.h"
#include "itkNumericTraits.h"

#include <iostream>
#include <string>

//-------------------------------------------------------------------------------------

/** run: A macro to call a function. */
#define run( function, type, dim ) \
if ( ComponentType == #type && Dimension == dim ) \
{ \
  typedef itk::Image< type, dim >   ImageType; \
  function< ImageType >( inputFileNames, maskFileName1, maskFileName2, t1, t2 ); \
  supported = true; \
}

#define run2( function, type, dim ) \
if ( ComponentType == #type && Dimension == dim ) \
{ \
  typedef itk::Image< type, dim >   ImageType; \
  function< ImageType >( inputFileNames, label ); \
  supported = true; \
}

//-------------------------------------------------------------------------------------

/** Declare PrintHelp. */
void PrintHelp( void );

template< class TImage>
void ComputeOverlapOld(
  const std::vector<std::string> & inputFileNames,
  const std::string & maskFileName1,
  const std::string & maskFileName2,
  const unsigned int & t1,
  const unsigned int & t2 );

template< class TImage>
void ComputeOverlap2(
  const std::vector<std::string> & inputFileNames,
  const unsigned int & label );

//-------------------------------------------------------------------------------------

int main( int argc, char ** argv )
{
  /** Check arguments for help. */
  if ( argc < 3 )
  {
    PrintHelp();
    return 1;
  }

  /** Create a command line argument parser. */
  itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
  parser->SetCommandLineArguments( argc, argv );

  /** Get arguments. */
  std::vector<std::string> inputFileNames;
  bool retin = parser->GetCommandLineArgument( "-in", inputFileNames );

  std::string maskFileName1 = "";
  bool retmask1 = parser->GetCommandLineArgument( "-mask1", maskFileName1 );
  std::string maskFileName2 = "";
  bool retmask2 = parser->GetCommandLineArgument( "-mask2", maskFileName2 );

  unsigned int t1 = 0;
  bool rett1 = parser->GetCommandLineArgument( "-t1", t1 );
  unsigned int t2 = 0;
  bool rett2 = parser->GetCommandLineArgument( "-t2", t2 );

  unsigned int label = 0; // default all labels
  bool retlabel = parser->GetCommandLineArgument( "-l", label );

  /** Checks. */
  if ( !retin || inputFileNames.size() != 2 )
  {
    std::cerr << "ERROR: You should specify two input file names with \"-in\"." << std::endl;
    return 1;
  }

  /** Determine image properties. */
  std::string ComponentType = "float";
  std::string PixelType = "VECTOR";
  unsigned int Dimension = 2;
  unsigned int NumberOfComponents = Dimension;
  std::vector<unsigned int> imagesize( Dimension, 0 );
  int retgip = GetImageProperties(
    inputFileNames[ 0 ],
    PixelType,
    ComponentType,
    Dimension,
    NumberOfComponents,
    imagesize );
  if ( retgip != 0 )
  {
    return 1;
  }

  /** Checks. */
  if ( NumberOfComponents > 1 )
  {
    std::cerr << "Vector images are not supported!" << std::endl;
    return 1;
  }

  /** Get rid of the possible "_" in ComponentType. */
  ReplaceUnderscoreWithSpace( ComponentType );

  /** Run the program. */
  bool supported = false;
  try
  {
    if ( retlabel )
    {
      run2( ComputeOverlap2, char, 2 );
      run2( ComputeOverlap2, short, 2 );

      run2( ComputeOverlap2, char, 3 );
      run2( ComputeOverlap2, unsigned char, 3 );
      run2( ComputeOverlap2, short, 3 );
      run2( ComputeOverlap2, unsigned short, 3 );
    }
    else
    {
      run( ComputeOverlapOld, char, 2 );
      run( ComputeOverlapOld, short, 2 );

      run( ComputeOverlapOld, char, 3 );
      run( ComputeOverlapOld, unsigned char, 3 );
      run( ComputeOverlapOld, short, 3 );
      run( ComputeOverlapOld, unsigned short, 3 );
    }
  }
  catch ( itk::ExceptionObject &e )
  {
    std::cerr << "Caught ITK exception: " << e << std::endl;
    return 1;
  }
  if ( !supported )
  {
    std::cerr << "ERROR: this combination of pixeltype and dimension is not "
      << "supported!" << std::endl;
    std::cerr
      << "pixel (component) type = " << ComponentType
      << " ; dimension = " << Dimension
      << std::endl;
    std::cerr << "Call \"pxcomputeoverlap --help\" to get a list "
      << "of supported images." << std::endl;
    return 1;
  }

  /** End program. */
  return 0;

} // end main


/**
 * ******************* PrintHelp *******************
 */

void PrintHelp( void )
{
  std::cout << "Usage:" << std::endl << "pxcomputeoverlap\n";
  std::cout << "This program computes the overlap of two binary images.\n"
    << "Masks of a valid region (for example an US beam) are also taken into account.\n"
    << "If the images are not binary, you must specify a threshold value.\n";
  std::cerr << "\nThe results is computed as:\n";
  std::cerr << "   2 * L1( (im1 AND mask2) AND (im2 AND mask1) )\n";
  std::cerr << "  ----------------------------------------------\n";
  std::cerr << "       L1(im1 AND mask2) + L1(im2 AND mask1)    \n" << std::endl;
  std::cout << "  -in      inputFilename1 inputFilename2\n";
  std::cout << "  [-mask1] maskFilename1\n";
  std::cout << "  [-mask2] maskFilename2\n";
  std::cout << "  [-t1]    threshold1\n";
  std::cout << "  [-t2]    threshold2\n";
  std::cout << "  [-l]     alternative implementation using a label value\n";
  std::cout << "Supported: 2D, 3D, (unsigned) char, (unsigned) short" << std::endl;

} // end PrintHelp()


/**
 * ******************* ComputeOverlapOld *******************
 */

template< class TImage>
void ComputeOverlapOld(
  const std::vector<std::string> & inputFileNames,
  const std::string & maskFileName1,
  const std::string & maskFileName2,
  const unsigned int & t1,
  const unsigned int & t2 )
{
  /** Some typedef's. */
  typedef TImage                                      ImageType;
  typedef typename ImageType::Pointer                 ImagePointer;
  typedef typename ImageType::PixelType               PixelType;
  typedef itk::ImageFileReader<ImageType>             ImageReaderType;
  typedef typename ImageReaderType::Pointer           ImageReaderPointer;
  typedef itk::AndImageFilter<
    ImageType, ImageType, ImageType>                  AndFilterType;
  typedef typename AndFilterType::Pointer             AndFilterPointer;
  typedef itk::ImageRegionConstIterator<ImageType>    IteratorType;
  typedef itk::ThresholdLabelerImageFilter<
    ImageType, ImageType>                             ThresholdFilterType;
  typedef typename ThresholdFilterType::Pointer       ThresholdFilterPointer;
  typedef typename ThresholdFilterType::ThresholdVector ThresholdVectorType;

  /**
   * Setup pipeline
   */

  /** Create readers and an AND filter. */
  ImageReaderPointer reader1 = ImageReaderType::New();
  reader1->SetFileName( inputFileNames[ 0 ].c_str() );
  ImageReaderPointer reader2 = ImageReaderType::New();
  reader2->SetFileName( inputFileNames[ 1 ].c_str() );
  AndFilterPointer finalANDFilter = AndFilterType::New();

  /** Create images, threshold filters, and threshold vectors. */
  ImagePointer im1 = 0;
  ImagePointer im2 = 0;
  ThresholdFilterPointer thresholder1 = 0;
  ThresholdFilterPointer thresholder2 = 0;
  ThresholdVectorType thresholdVector1( 2 );
  ThresholdVectorType thresholdVector2( 2 );

  /** If there is a threshold given for image1, use it. */
  if ( t1 != 0 )
  {
    thresholder1 = ThresholdFilterType::New();
    thresholdVector1[ 0 ] = t1;
    thresholdVector1[ 1 ] = itk::NumericTraits<PixelType>::max();
    thresholder1->SetThresholds( thresholdVector1 );
    thresholder1->SetInput( reader1->GetOutput() );
    im1 = thresholder1->GetOutput();
  }
  /** Otherwise, just take the input image1. */
  else
  {
    im1 = reader1->GetOutput();
  }

  /** If there is a threshold given for image2, use it. */
  if ( t2 != 0 )
  {
    thresholder2 = ThresholdFilterType::New();
    thresholdVector2[ 0 ] = t2;
    thresholdVector2[ 1 ] = itk::NumericTraits<PixelType>::max();
    thresholder2->SetThresholds( thresholdVector2 );
    thresholder2->SetInput( reader2->GetOutput() );
    im2 = thresholder2->GetOutput();
  }
  else
  {
    im2 = reader2->GetOutput();
  }

  /** Create readers for the masks and AND filters. */
  ImageReaderPointer maskReader1 = 0;
  ImageReaderPointer maskReader2 = 0;
  AndFilterPointer im2ANDmask1Filter = 0;
  AndFilterPointer im1ANDmask2Filter = 0;

  /** If there is a mask given for image1, use it on image2. */
  if ( maskFileName1 != "" )
  {
    maskReader1 = ImageReaderType::New();
    maskReader1->SetFileName( maskFileName1.c_str() );
    im2ANDmask1Filter = AndFilterType::New();
    im2ANDmask1Filter->SetInput1( im2 );
    im2ANDmask1Filter->SetInput2( maskReader1->GetOutput() );
    finalANDFilter->SetInput1( im2ANDmask1Filter->GetOutput() );
  }
  /** Otherwise, just use image2. */
  else
  {
    finalANDFilter->SetInput1( im2 );
  }

  /** If there is a mask given for image2, use it on image1. */
  if ( maskFileName2 != "" )
  {
    maskReader2 = ImageReaderType::New();
    maskReader2->SetFileName( maskFileName2.c_str() );
    im1ANDmask2Filter = AndFilterType::New();
    im1ANDmask2Filter->SetInput1( im1 );
    im1ANDmask2Filter->SetInput2( maskReader2->GetOutput() );
    finalANDFilter->SetInput2( im1ANDmask2Filter->GetOutput() );
  }
  /** Otherwise, just use image1. */
  else
  {
    finalANDFilter->SetInput2( im1 );
  }

  /** UPDATE!
   *
   * Call the update of the final filter, in order to execute the whole pipeline.
   */

  try
  {
    finalANDFilter->Update();
  }
  catch ( itk::ExceptionObject & excp )
  {
    std::cerr << excp << std::endl;
    throw excp;
  }

  /**
   * Now calculate the L1-norm.
   */

  /** Create iterators. */
  IteratorType iteratorA( finalANDFilter->GetInput(1),
    finalANDFilter->GetInput(1)->GetLargestPossibleRegion() );
  IteratorType iteratorB( finalANDFilter->GetInput(0),
    finalANDFilter->GetInput(0)->GetLargestPossibleRegion() );
  IteratorType iteratorC( finalANDFilter->GetOutput(),
    finalANDFilter->GetOutput()->GetLargestPossibleRegion() );

  /** Determine size of first object. */
  long long sumA = 0;
  for ( iteratorA.GoToBegin(); !iteratorA.IsAtEnd(); ++iteratorA )
  {
    if ( iteratorA.Value() )
    {
      ++sumA;
    }
  }
  std::cout << "Size of first object: " << sumA << std::endl;

  /** Determine size of second object. */
  long long sumB = 0;
  for ( iteratorB.GoToBegin(); !iteratorB.IsAtEnd(); ++iteratorB )
  {
    if ( iteratorB.Value() )
    {
      ++sumB;
    }
  }
  std::cout << "Size of second object: " << sumB << std::endl;

  /** Determine size of cross-section. */
  long long sumC = 0;
  for ( iteratorC.GoToBegin(); !iteratorC.IsAtEnd(); ++iteratorC )
  {
    if ( iteratorC.Value() )
    {
      ++sumC;
    }
  }
  std::cout << "Size of cross-section of both objects: " << sumC << std::endl;

  /** Calculate the overlap. */
  double overlap;
  if ( ( sumA + sumB ) == 0 )
  {
    overlap = 0;
  }
  else
  {
    overlap = static_cast<double>( 2 * sumC ) / static_cast<double>( sumA + sumB );
  }

  /** Format the output and show overlap. */
  std::cout << std::fixed << std::showpoint;
  std::cout << "Overlap: " << overlap << std::endl;

} // end ComputeOverlapOld


/**
 * ******************* ComputeOverlap2 *******************
 */


template< class TImage>
void ComputeOverlap2(
  const std::vector<std::string> & inputFileNames,
  const unsigned int & label )
{
  /** Some typedef's. */
  typedef TImage                                      ImageType;
  typedef typename ImageType::Pointer                 ImagePointer;
  typedef typename ImageType::PixelType               PixelType;
  typedef itk::ImageFileReader<ImageType>             ImageReaderType;
  typedef typename ImageReaderType::Pointer           ImageReaderPointer;
  typedef itk::ImageRegionConstIterator<ImageType>    IteratorType;

  /**
   * Setup pipeline
   */

  /** Create readers. */
  ImageReaderPointer reader1 = ImageReaderType::New();
  reader1->SetFileName( inputFileNames[ 0 ].c_str() );
  reader1->Update();
  ImageReaderPointer reader2 = ImageReaderType::New();
  reader2->SetFileName( inputFileNames[ 1 ].c_str() );
  reader2->Update();

  ImagePointer imA = reader1->GetOutput();
  ImagePointer imB = reader2->GetOutput();

  /** Create iterators. */
  IteratorType itA( imA, imA->GetLargestPossibleRegion() );
  IteratorType itB( imB, imB->GetLargestPossibleRegion() );
  itA.GoToBegin();
  itB.GoToBegin();

  /** Determine size of objects. *
  long long sumA, sumB, sumC;
  sumA = sumB = sumC = 0;
  while ( !itA.IsAtEnd() )
  {
    PixelType A = itA.Value();
    PixelType B = itB.Value();
    if ( A == label ) ++sumA;
    if ( B == label ) ++sumB;
    if ( A == label && B == label ) ++sumC;

    ++itA; ++itB;
  }*/

  // Assuming positive labels only.
  std::size_t maxNumberOfLabels = 10;
  if ( label != 0 ) maxNumberOfLabels = 1;

  /** Determine size of objects. */
  std::vector<long long> sumA( maxNumberOfLabels, 0 );
  std::vector<long long> sumB( maxNumberOfLabels, 0 );
  std::vector<long long> sumC( maxNumberOfLabels, 0 );
  while ( !itA.IsAtEnd() )
  {
    PixelType A = itA.Value();
    PixelType B = itB.Value();

    if ( label == 0 )
    {
      ++sumA[ A ];
      ++sumB[ B ];
      if ( A == B  ) ++sumC[ A ];
    }
    else
    {
      if ( A == label ) ++sumA[ 0 ];
      if ( B == label ) ++sumB[ 0 ];
      if ( A == label && B == label ) ++sumC[ 0 ];
    }

    ++itA; ++itB;
  }

  /** Calculate the overlap. */
  std::vector<double> overlap( maxNumberOfLabels, 0.0 );
  for ( std::size_t i = 0; i < maxNumberOfLabels; ++i )
  {
    const long long sumAB = sumA[ i ] + sumB[ i ];
    if ( sumAB == 0 ) overlap[ i ] = 0.0;
    else
    {
      overlap[ i ] = static_cast<double>( 2 * sumC[ i ] )
        / static_cast<double>( sumAB );
    }
  }

  /** Format the output and show overlap. */
  if ( label == 0 )
  {
    std::cout << "label:   ";
    for ( std::size_t i = 0; i < maxNumberOfLabels; ++i )
    {
      std::cout << i << " ";
    }
    std::cout << std::endl;
  }
  else
  {
    std::cout << "label:   " << label << std::endl;
  }

  std::cout << "|A|:     ";
  for ( std::size_t i = 0; i < maxNumberOfLabels; ++i )
  {
    std::cout << sumA[ i ] << " ";
  }
  std::cout << std::endl;

  std::cout << "|B|:     ";
  for ( std::size_t i = 0; i < maxNumberOfLabels; ++i )
  {
    std::cout << sumB[ i ] << " ";
  }
  std::cout << std::endl;

  std::cout << "|A & B|: ";
  for ( std::size_t i = 0; i < maxNumberOfLabels; ++i )
  {
    std::cout << sumC[ i ] << " ";
  }
  std::cout << std::endl;

  std::cout << "overlap: ";
  std::cout << std::fixed << std::showpoint;
  for ( std::size_t i = 0; i < maxNumberOfLabels; ++i )
  {
    std::cout << overlap[ i ] << " ";
  }
  std::cout << std::endl;

} // end ComputeOverlap2()

