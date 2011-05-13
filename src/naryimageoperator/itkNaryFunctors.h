#ifndef __itkNaryFunctors_h
#define __itkNaryFunctors_h

#include "itkNumericTraits.h"

namespace itk {

namespace Functor {


template< class TInput, class TOutput = TInput >
class NaryADDITION
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryADDITION() {};
  ~NaryADDITION() {};
  bool operator!=( const NaryADDITION & ) const{ return false; }
  bool operator==( const NaryADDITION & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = NumericTraits< ScalarRealType >::Zero;
    for ( unsigned int i = 0; i < B.size(); i++ )
    {
      result += B[ i ];
    }
    return static_cast< TOutput >( result );
  }
};

template< class TInput, class TOutput = TInput >
class NaryMEAN
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryMEAN() {};
  ~NaryMEAN() {};
  bool operator!=( const NaryMEAN & ) const{ return false; }
  bool operator==( const NaryMEAN & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = NumericTraits< ScalarRealType >::Zero;
    for ( unsigned int i = 0; i < B.size(); i++ )
    {
      result += B[ i ];
    }
    return static_cast< TOutput >( result / B.size() );
  }
};

template< class TInput, class TOutput = TInput >
class NaryMINUS
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryMINUS() {};
  ~NaryMINUS() {};
  bool operator!=( const NaryMINUS & ) const{ return false; }
  bool operator==( const NaryMINUS & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = static_cast< ScalarRealType >( B[ 0 ] );
    for ( unsigned int i = 1; i < B.size(); i++ )
    {
      result -= B[ i ];
    }
    return static_cast< TOutput >( result );
  }
};


template< class TInput, class TOutput = TInput >
class NaryTIMES
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryTIMES() {};
  ~NaryTIMES() {};
  bool operator!=( const NaryTIMES & ) const{ return false; }
  bool operator==( const NaryTIMES & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = NumericTraits< ScalarRealType >::One;
    for ( unsigned int i = 0; i < B.size(); i++ )
    {
      result *= B[ i ];
    }
    return static_cast< TOutput >( result );
  }
};


template< class TInput, class TOutput = TInput >
class NaryDIVIDE
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryDIVIDE() {};
  ~NaryDIVIDE() {};
  bool operator!=( const NaryDIVIDE & ) const{ return false; }
  bool operator==( const NaryDIVIDE & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = static_cast< ScalarRealType >( B[ 0 ] );
    for ( unsigned int i = 1; i < B.size(); i++ )
    {
      result = B[ i ] != 0 ? result / B[ i ] : result;
    }
    return static_cast< TOutput >( result );
  }
};

template< class TInput, class TOutput = TInput >
class NaryMAXIMUM
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryMAXIMUM() {};
  ~NaryMAXIMUM() {};
  bool operator!=( const NaryMAXIMUM & ) const{ return false; }
  bool operator==( const NaryMAXIMUM & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    AccumulateType result = static_cast< AccumulateType >( B[ 0 ] );
    for ( unsigned int i = 1; i < B.size(); i++ )
    {
      result = vnl_math_max( result, B[ i ] );
    }
    return static_cast< TOutput >( result );
  }
};


template< class TInput, class TOutput = TInput >
class NaryMINIMUM
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryMINIMUM() {};
  ~NaryMINIMUM() {};
  bool operator!=( const NaryMINIMUM & ) const{ return false; }
  bool operator==( const NaryMINIMUM & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    AccumulateType result = static_cast< AccumulateType >( B[ 0 ] );
    for ( unsigned int i = 1; i < B.size(); i++ )
    {
      result = vnl_math_min( result, B[ i ] );
    }
    return static_cast< TOutput >( result );
  }
};


template< class TInput, class TOutput = TInput >
class NaryABSOLUTEDIFFERENCE
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryABSOLUTEDIFFERENCE() {};
  ~NaryABSOLUTEDIFFERENCE() {};
  bool operator!=( const NaryABSOLUTEDIFFERENCE & ) const{ return false; }
  bool operator==( const NaryABSOLUTEDIFFERENCE & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = static_cast< ScalarRealType >( B[ 0 ] );
    for ( unsigned int i = 1; i < B.size(); i++ )
    {
      result -= B[ i ];
    }
    return static_cast< TOutput >( result > 0.0 ? result : -result );
  }
};


template< class TInput, class TOutput = TInput >
class NaryNARYMAGNITUDE
{
public:
  typedef typename NumericTraits< TInput >::AccumulateType AccumulateType;
  typedef typename NumericTraits< TInput >::ScalarRealType ScalarRealType;
  NaryNARYMAGNITUDE() {};
  ~NaryNARYMAGNITUDE() {};
  bool operator!=( const NaryNARYMAGNITUDE & ) const{ return false; }
  bool operator==( const NaryNARYMAGNITUDE & other ) const{ return !(*this != other); }
  inline TOutput operator()( const std::vector< TInput > & B ) const
  {
    ScalarRealType result = NumericTraits< ScalarRealType >::Zero;
    for ( unsigned int i = 0; i < B.size(); i++ )
    {
      result += B[ i ] * B[ i ];
    }
    return static_cast< TOutput >( vcl_sqrt( result ) );
  }
};

} // end namespace Functor


} // end namespace itk


#endif // end #ifndef __itkNaryFunctors_h
