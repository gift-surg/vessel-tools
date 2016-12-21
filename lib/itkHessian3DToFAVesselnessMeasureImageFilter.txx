#ifndef ITKHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_TXX
#define ITKHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_TXX

#include "itkHessian3DToFAVesselnessMeasureImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkMath.h>
#include <itkImageRegionConstIterator.h>
#include <vnl/vnl_math.h>

#define EPS  1e-03
namespace itk {

template< typename TPixel >
Hessian3DToFAVesselnessMeasureImageFilter< TPixel >
::Hessian3DToFAVesselnessMeasureImageFilter()
{
  m_UseDiffusion = true;
}

template< typename TPixel >
void Hessian3DToFAVesselnessMeasureImageFilter< TPixel >
::GenerateData()
{
  typename OutputImageType::Pointer output = this->GetOutput();
    this->AllocateOutputs();

  // walk the region of eigen values
  ImageRegionConstIterator<InputImageType> it;
  it = ImageRegionConstIterator<InputImageType>(
        this->GetInput(), this->GetInput()->GetRequestedRegion());

  ImageRegionIterator<OutputImageType> oit;
  oit = ImageRegionIterator<OutputImageType>(output,
                                             output->GetRequestedRegion());
  //Initialise analyser
  EigenAnalysisType  eig;
  EigenVectorType tmpMatrix;
  EigenValueType eigenVal;

  eig.SetDimension( ImageDimension );
  eig.SetOrderEigenMagnitudes( true );
  eig.SetOrderEigenValues( false );
  eigenVal.Fill(0);

  oit.GoToBegin();
  it.GoToBegin();
  while (!oit.IsAtEnd())
  {
    SymmetricSecondRankTensor<double, 3> tmpTensor = it.Get();
    tmpMatrix[0][0] = tmpTensor[0];
    tmpMatrix[0][1] = tmpTensor[1];
    tmpMatrix[0][2] = tmpTensor[2];
    tmpMatrix[1][0] = tmpTensor[1];
    tmpMatrix[1][1] = tmpTensor[3];
    tmpMatrix[1][2] = tmpTensor[4];
    tmpMatrix[2][0] = tmpTensor[2];
    tmpMatrix[2][1] = tmpTensor[4];
    tmpMatrix[2][2] = tmpTensor[5];

    eig.ComputeEigenValues( tmpMatrix, eigenVal );

    double fr = 0.01;
    if (eigenVal[0] * eigenVal[1] * eigenVal[2] < 0)
    {
      tmpMatrix[0][0] = tmpTensor[0]+fr;
      tmpMatrix[0][1] = tmpTensor[1];
      tmpMatrix[0][2] = tmpTensor[2];
      tmpMatrix[1][0] = tmpTensor[1];
      tmpMatrix[1][1] = tmpTensor[3]+fr;
      tmpMatrix[1][2] = tmpTensor[4];
      tmpMatrix[2][0] = tmpTensor[2];
      tmpMatrix[2][1] = tmpTensor[4];
      tmpMatrix[2][2] = tmpTensor[5]+fr;
    }
  eig.ComputeEigenValues( tmpMatrix, eigenVal );
    // Find the smallest eigenvalue
    double smallest = vnl_math_abs( eigenVal[0] );
    double Lambda1 = eigenVal[0];

    for ( unsigned int i=1; i <=2; i++ )
    {
      if ( vnl_math_abs( eigenVal[i] ) < smallest )
      {
        Lambda1 = eigenVal[i];
        smallest = vnl_math_abs( eigenVal[i] );
      }
    }
    // Find the largest eigenvalue
    double largest = vnl_math_abs( eigenVal[0] );
    double Lambda3 = eigenVal[0];

    for ( unsigned int i=1; i <=2; i++ )
    {
      if (  vnl_math_abs( eigenVal[i] ) > largest  )
      {
        Lambda3 = eigenVal[i];
        largest = vnl_math_abs( eigenVal[i] );
      }
    }

    //  find Lambda2 so that |Lambda1| < |Lambda2| < |Lambda3|
    double Lambda2 = eigenVal[0];

    for ( unsigned int i=0; i <=2; i++ )
    {
      if ( eigenVal[i] != Lambda1 && eigenVal[i] != Lambda3 )
      {
        Lambda2 = eigenVal[i];
        break;
      }
    }
    double S = vcl_sqrt( Lambda2*Lambda2 + Lambda1*Lambda1 + Lambda3*Lambda3 );
    double factor = (1-vcl_exp( -1.0 * (( vnl_math_sqr( S )) / ( 2.0 * ( 25.0))))
                     );

    //Signs of the lambdas can be checked from here for both diffusion and hessian
    // at the moment there is no checking on the fact that lambdas > 0 for diffusion,
    //which is mathematically wrong.
    if ( Lambda2 >= 0.0 ||  Lambda3 >= 0.0 ||
         vnl_math_abs( Lambda2 ) < EPS  ||
         vnl_math_abs( Lambda3 ) < EPS )
    {
      oit.Set( NumericTraits< OutputPixelType >::Zero );
    }
    else
    {
      if (m_UseDiffusion)
      {
        double tmp_min = Lambda1;
        Lambda1 = -1.0 / Lambda3;
        Lambda2 = -1.0 / Lambda2;
        Lambda3 = -1.0 / tmp_min;
      }

      OutputPixelType lambda1 = vnl_math_abs(Lambda1);
      OutputPixelType lambda2 = vnl_math_abs(Lambda2);
      OutputPixelType lambda3 = vnl_math_abs(Lambda3);
      OutputPixelType trace = (lambda1+lambda2+lambda3)/3;
      factor =1;
      OutputPixelType value = factor * K * (vcl_sqrt(vnl_math_sqr(lambda1-trace) +
                                                     vnl_math_sqr(lambda2-trace) +
                                                     vnl_math_sqr(lambda3-trace))) /
          vcl_sqrt(vnl_math_sqr(lambda1) +
                   vnl_math_sqr(lambda1) +
                   vnl_math_sqr(lambda3));
      oit.Set( value );
    }
//    oit.Set(Lambda1);
    ++oit;
    ++it;
  }
}

/* ---------------------------------------------------------------------
   PrintSelf method
   --------------------------------------------------------------------- */
template< typename TPixel >
void
Hessian3DToFAVesselnessMeasureImageFilter< TPixel >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

}
#endif //ITKHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_TXX
