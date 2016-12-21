#ifndef __itkHessianSmoothed3DToVesselnessMeasureImageVectorFilter_txx
#define __itkHessianSmoothed3DToVesselnessMeasureImageVectorFilter_txx

#include "itkHessianSmoothed3DToVesselnessMeasureImageVectorFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "vnl/vnl_math.h"

#define EPSILON  1e-03

namespace itk
{

/**
 * Constructor
 */
template < typename TPixel >
HessianSmoothed3DToVesselnessMeasureImageVectorFilter< TPixel >
::HessianSmoothed3DToVesselnessMeasureImageVectorFilter()
{
  m_Alpha = 0.5;
  m_Beta  = 0.5;
  m_Gamma = 5.0;

  m_C = 10e-6;



  // By default, scale the vesselness measure by the largest
  // eigen value
  m_ScaleVesselnessMeasure  = true;

  //By default, vessels are bright
  m_BrightVessels = true;

}


template < typename TPixel >
void
HessianSmoothed3DToVesselnessMeasureImageVectorFilter< TPixel >
::GenerateData()
{
  itkDebugMacro(
        << "HessianSmoothed3DToVesselnessMeasureImageVectorFilter generating data ");
  this->AllocateOutputs();
  ImageRegionConstIterator<InputImageType> it;
  it = ImageRegionConstIterator<InputImageType>(
      this->GetInput(), this->GetInput()->GetRequestedRegion());

  typename OutputImageType::Pointer output = this->GetOutput();

 // InputRegionType region = this->GetInput()->GetLargestPossibleRegion();

  EigenAnalysisFilterType eig;
  eig.SetDimension( ImageDimension );
  eig.SetOrderEigenMagnitudes( true );
  eig.SetOrderEigenValues( false );

  EigenMatrixType matrix, tmpMatrix;
  EigenValueType eigenValue;
  EigenIteratorType eit;
  matrix.Fill(0);
  eigenValue.Fill(0);
  // Allocate Eigen matrix
  if ( m_ComputeEigenVectors )
  {
    m_EigenMatrixImage = EigenMatrixImageType::New();
    m_EigenMatrixImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
    m_EigenMatrixImage->CopyInformation( this->GetInput() );
    m_EigenMatrixImage->Allocate();
    eit = EigenIteratorType( m_EigenMatrixImage,
                             m_EigenMatrixImage->GetLargestPossibleRegion() );
    eit.GoToBegin();
  }

  ImageRegionIterator<OutputImageType> oit;
  oit = ImageRegionIterator<OutputImageType>(output,
                                             output->GetRequestedRegion());

  oit.GoToBegin();
  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    EigenMatrixType eigenMatrix;
    eigenMatrix.Fill(0);
    SymmetricSecondRankTensor<double, 3> tmpTensor = it.Get();
    //ImgOne values
    tmpMatrix[0][0] = tmpTensor[0];
    tmpMatrix[0][1] = tmpTensor[1];
    tmpMatrix[0][2] = tmpTensor[2];
    tmpMatrix[1][0] = tmpTensor[1];
    tmpMatrix[1][1] = tmpTensor[3];
    tmpMatrix[1][2] = tmpTensor[4];
    tmpMatrix[2][0] = tmpTensor[2];
    tmpMatrix[2][1] = tmpTensor[4];
    tmpMatrix[2][2] = tmpTensor[5];
    eig.ComputeEigenValuesAndVectors( tmpMatrix, eigenValue, matrix );

    unsigned int index1_one, index2_one, index3_one;
    this->OrderEigenValuesByMagnitude(eigenValue,index1_one,index2_one,index3_one);
    double Lambda1 = eigenValue[index1_one];
    double Lambda2 = eigenValue[index2_one];
    double Lambda3 = eigenValue[index3_one];
    // Find the smallest eigenvalue
 /*   double smallest = vnl_math_abs( eigenValue[0] );
    double Lambda1 = eigenValue[0];

    for ( unsigned int i=1; i <=2; i++ )
    {
      if ( vnl_math_abs( eigenValue[i] ) < smallest )
      {
        Lambda1 = eigenValue[i];
        smallest = vnl_math_abs( eigenValue[i] );
      }
    }

    // Find the largest eigenvalue
    double largest = vnl_math_abs( eigenValue[0] );
    double Lambda3 = eigenValue[0];

    for ( unsigned int i=1; i <=2; i++ )
    {
      if (  vnl_math_abs( eigenValue[i] ) > largest  )
      {
        Lambda3 = eigenValue[i];
        largest = vnl_math_abs( eigenValue[i] );
      }
    }


    //  find Lambda2 so that |Lambda1| < |Lambda2| < |Lambda3|
    double Lambda2 = eigenValue[0];

    for ( unsigned int i=0; i <=2; i++ )
    {
      if ( eigenValue[i] != Lambda1 && eigenValue[i] != Lambda3 )
      {
        Lambda2 = eigenValue[i];
        break;
      }
    }
*/
    //Now we need to check for bright dark things
    bool zeroSet = false;     //Flag to notify that the zero was set or not set.
    if (m_BrightVessels)
    {
      if ( Lambda2 >= 0.0 ||  Lambda3 >= 0.0 ||
           vnl_math_abs( Lambda2) < EPSILON  ||
           vnl_math_abs( Lambda3 ) < EPSILON )
      {
        oit.Set( NumericTraits< OutputPixelType >::Zero );
        zeroSet = true;
      }
    }
    else //Dark structures
    {
      if ( Lambda2 <= 0.0 ||  Lambda3 <= 0.0 ||
           vnl_math_abs( Lambda2) < EPSILON  ||
           vnl_math_abs( Lambda3 ) < EPSILON )
      {
        oit.Set( NumericTraits< OutputPixelType >::Zero );
        zeroSet = true;
      }
    }

    /* Obsolete bit. Does not deal with dark/bright structures
    /*if ( Lambda2 >= 0.0 ||  Lambda3 >= 0.0 ||
         vnl_math_abs( Lambda2) < EPSILON  ||
         vnl_math_abs( Lambda3 ) < EPSILON )
    {
      oit.Set( NumericTraits< OutputPixelType >::Zero );
    }*/
    if (!zeroSet)
    {

      double Lambda1Abs = vnl_math_abs( Lambda1 );
      double Lambda2Abs = vnl_math_abs( Lambda2 );
      double Lambda3Abs = vnl_math_abs( Lambda3 );

      double Lambda1Sqr = vnl_math_sqr( Lambda1 );
      double Lambda2Sqr = vnl_math_sqr( Lambda2 );
      double Lambda3Sqr = vnl_math_sqr( Lambda3 );

      double AlphaSqr = vnl_math_sqr( m_Alpha );
      double BetaSqr = vnl_math_sqr( m_Beta );
      double GammaSqr = vnl_math_sqr( m_Gamma );

      double A  = Lambda2Abs / Lambda3Abs;
      double B  = Lambda1Abs / vcl_sqrt ( vnl_math_abs( Lambda2 * Lambda3 ));
      double S  = vcl_sqrt( Lambda1Sqr + Lambda2Sqr + Lambda3Sqr );

      double vesMeasure_1  =
          ( 1 - vcl_exp(-1.0*(( vnl_math_sqr(A) ) / ( 2.0 * ( AlphaSqr)))));

      double vesMeasure_2  =
          vcl_exp ( -1.0 * ((vnl_math_sqr( B )) /  ( 2.0 * (BetaSqr))));

      double vesMeasure_3  = 1;
      ( 1 - vcl_exp( -1.0 * (( vnl_math_sqr( S )) / ( 2.0 * ( GammaSqr)))));

      double vesMeasure_4  =
          vcl_exp ( -1.0 * ( 2.0 * vnl_math_sqr( m_C )) /
                    ( Lambda2Abs * (Lambda3Sqr)));

      double vesselnessMeasure =
          vesMeasure_1 * vesMeasure_2 * vesMeasure_3 * vesMeasure_4;

      if(  m_ScaleVesselnessMeasure )
      {
        oit.Set( static_cast< OutputPixelType >(
                   Lambda3Abs*vesselnessMeasure ) );
      }
      else
      {
        oit.Set( static_cast< OutputPixelType >( vesselnessMeasure ) );
      }
      if ( m_ComputeEigenVectors )
      {

        eigenMatrix[index1_one][0] = matrix[0][0];
        eigenMatrix[index1_one][1] = matrix[0][1];
        eigenMatrix[index1_one][2] = matrix[0][2];
        eigenMatrix[index2_one][0] = matrix[1][0];
        eigenMatrix[index2_one][1] = matrix[1][1];
        eigenMatrix[index2_one][2] = matrix[1][2];
        eigenMatrix[index3_one][0] = matrix[2][0];
        eigenMatrix[index3_one][1] = matrix[2][1];
        eigenMatrix[index3_one][2] = matrix[2][2];
      }
    }
    if ( m_ComputeEigenVectors )
    {
      eit.Set( eigenMatrix );
      ++eit;
    }
    ++it;
    ++oit;
  }

}

template< typename TPixel >
void
HessianSmoothed3DToVesselnessMeasureImageVectorFilter< TPixel >
::OrderEigenValuesByMagnitude(EigenValueType eigenVal, unsigned int &indexone,
                                 unsigned int &indextwo, unsigned int &indexthree)
{
  double smallest = vnl_math_abs( eigenVal[0] );
  double Lambda1 = eigenVal[0];
  indexone = 0, indextwo = 0, indexthree = 0;

  for ( unsigned int i=1; i <=2; i++ )
  {
    if ( vnl_math_abs( eigenVal[i] ) < smallest )
    {
      Lambda1 = eigenVal[i];
      smallest = vnl_math_abs( eigenVal[i] );
      indexone = i;
    }
  }
//    // Find the largest eigenvalue
  double largest = vnl_math_abs( eigenVal[0] );
  double Lambda3 = eigenVal[0];

  for ( unsigned int i=1; i <=2; i++ )
  {
    if (  vnl_math_abs( eigenVal[i] ) > largest  )
    {
      Lambda3 = eigenVal[i];
      largest = vnl_math_abs( eigenVal[i] );
      indexthree = i;
    }
  }

  //  find Lambda2 so that |Lambda1| < |Lambda2| < |Lambda3|
//  double Lambda2 = eigenVal[0];

  for ( unsigned int i=0; i <=2; i++ )
  {
    if ( eigenVal[i] != Lambda1 && eigenVal[i] != Lambda3 )
    {
      //Lambda2 = eigenVal[i];
      indextwo =i;
      break;
    }
  }
}

template < typename TPixel >
void
HessianSmoothed3DToVesselnessMeasureImageVectorFilter< TPixel >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Alpha: " << m_Alpha << std::endl;
  os << indent << "Beta:  " << m_Beta  << std::endl;
  os << indent << "Gamma: " << m_Gamma << std::endl;

  os << indent << "C: " << m_C << std::endl;
}


} // end namespace itk

#endif

