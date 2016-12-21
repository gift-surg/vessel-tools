#ifndef __itkMultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter_txx
#define __itkMultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter_txx

#include "itkMultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <vnl/vnl_math.h>

#define EPSILON  1e-03

namespace itk
{

/**
 * Constructor
 */
template <typename TInputImage, typename TOutputImage >
MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
<TInputImage,TOutputImage>
::MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter()
{
  m_SigmaMin = 0.2;
  m_SigmaMax = 2.0;
  m_ScaleMode = LINEAR;
  m_NumberOfSigmaSteps = 10;
  m_IsSigmaStepLog = true;
  m_ComputeMatrix =true;

  m_HessianFilter                = HessianFilterType::New();
  m_VesselnessFilter             = VesselnessFilterType::New();

  //Turn off vesselness measure scaling
  m_VesselnessFilter->SetScaleVesselnessMeasure( false );
  m_VesselnessFilter->SetComputeEigenVectors( m_ComputeMatrix );

  //Instantiate Update buffer
  m_UpdateBuffer                 = UpdateBufferType::New();
  m_EigenMatrixImage = EigenMatrixImageType::New();
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
<TInputImage,TOutputImage>
::AllocateUpdateBuffer()
{
  /* The update buffer looks just like the output and holds the best response
     in the  vesselness measure */

  typename TOutputImage::Pointer output = this->GetOutput();

  m_UpdateBuffer->SetSpacing(output->GetSpacing());
  m_UpdateBuffer->SetOrigin(output->GetOrigin());
  m_UpdateBuffer->SetLargestPossibleRegion(output->GetLargestPossibleRegion());
  m_UpdateBuffer->SetRequestedRegion(output->GetRequestedRegion());
  m_UpdateBuffer->SetBufferedRegion(output->GetBufferedRegion());
  m_UpdateBuffer->Allocate();

  EigenMatrixType p;
  p.Fill( 0 );

  m_EigenMatrixImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  m_EigenMatrixImage->CopyInformation( this->GetInput() );
  m_EigenMatrixImage->Allocate();
  m_EigenMatrixImage->FillBuffer( p );
}


template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
<TInputImage,TOutputImage>
::GenerateData()
{
  // Allocate the output
  this->GetOutput()->SetBufferedRegion(
        this->GetOutput()->GetRequestedRegion() );
  this->GetOutput()->Allocate();

  // Allocate the buffer
  AllocateUpdateBuffer();
  typename InputImageType::ConstPointer input = this->GetInput();
  this->m_HessianFilter->SetInput( input );
  this->m_HessianFilter->SetNormalizeAcrossScale( true );
  m_VesselnessFilter->SetBrightVessels(m_BrightVessels);
  double sigma = m_SigmaMin;

  int scaleLevel = 1;


  do {
//        std::cout << "Computing vesselness for scale with sigma= "
//                  << sigma << std::endl;

    m_HessianFilter->SetSigma( sigma );
    m_VesselnessFilter->SetInput ( m_HessianFilter->GetOutput() );
    m_VesselnessFilter->Update();
    this->UpdateMaximumResponse();

    sigma  = this->ComputeSigmaValue( scaleLevel );

    scaleLevel++;
  }while ( sigma <= m_SigmaMax );

  //Write out the best response to the output image
  ImageRegionIterator<UpdateBufferType>
      it(m_UpdateBuffer,m_UpdateBuffer->GetLargestPossibleRegion());
  it.GoToBegin();

  ImageRegionIterator<TOutputImage> oit(this->GetOutput(),
                                        this->GetOutput()->GetLargestPossibleRegion());
  oit.GoToBegin();

  while(!oit.IsAtEnd())
  {
    oit.Value() = static_cast< OutputPixelType >( it.Get() );
    ++oit;
    ++it;
  }
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
<TInputImage,TOutputImage>
::UpdateMaximumResponse()
{

  ImageRegionIterator<UpdateBufferType>
      oit(m_UpdateBuffer,m_UpdateBuffer->GetLargestPossibleRegion());

  oit.GoToBegin();

  typedef typename VesselnessFilterType::OutputImageType
      VesselnessOutputImageType;

  ImageRegionIterator<VesselnessOutputImageType>
      it(m_VesselnessFilter->GetOutput(),
         this->GetOutput()->GetLargestPossibleRegion());

  ImageRegionIterator<EigenMatrixImageType>
      mit( m_EigenMatrixImage, this->GetOutput()->GetLargestPossibleRegion() );

  ImageRegionIterator<EigenMatrixImageType>
      hit( m_VesselnessFilter->GetEigenMatrix(),
           this->GetOutput()->GetLargestPossibleRegion() );

  it.GoToBegin();
  oit.GoToBegin();
  mit.GoToBegin();
  hit.GoToBegin();

  while(!oit.IsAtEnd())
  {
    if( oit.Value() < it.Value() )
    {
      oit.Value() = it.Value();
      mit.Set( hit.Get() );
    }
    ++oit;
    ++it;
    ++mit;
    ++hit;
  }
}

template <typename TInputImage, typename TOutputImage >
double
MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
<TInputImage,TOutputImage>
::ComputeSigmaValue( int ScaleLevel )
{
  double stepSize = 0, sigma = 0;

  switch (m_ScaleMode)
  {
  case LINEAR:
  {
    stepSize = (m_SigmaMax-m_SigmaMin) / m_NumberOfSigmaSteps;
    if( stepSize <= 1e-10 )
    {
      stepSize = 1e-10;
    }
    sigma =  m_SigmaMin + (stepSize * ScaleLevel);
    break;
  }
  case EXPONENTIAL:
  {
    stepSize =
        ( vcl_log( m_SigmaMax )  - vcl_log( m_SigmaMin) ) / m_NumberOfSigmaSteps;
    if( stepSize <= 1e-10 )
    {
      stepSize = 1e-10;
    }
    sigma = ( vcl_exp( vcl_log (m_SigmaMin) + stepSize * ScaleLevel) );
    if (m_SigmaMax == m_SigmaMin)
      sigma +=0.1;
    break;
  }
  default:
  {
    std::cerr << "Error: Unknown scale mode option for the vesselness filter" << std::endl;
    return 1.0f;
  }
  }

  return sigma;
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SigmaMin:  " << m_SigmaMin << std::endl;
  os << indent << "SigmaMax:  " << m_SigmaMax  << std::endl;
}


} // end namespace itk

#endif

