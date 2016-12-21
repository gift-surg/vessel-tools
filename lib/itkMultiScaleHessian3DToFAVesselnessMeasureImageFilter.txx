#ifndef ITKMULTISCALEHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_TXX
#define ITKMULTISCALEHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_TXX

#include "itkMultiScaleHessian3DToFAVesselnessMeasureImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

namespace itk {

template <typename TInputImage, typename TOutputImage >
MultiScaleHessian3DToFAVesselnessMeasureImageFilter<TInputImage,TOutputImage>
::MultiScaleHessian3DToFAVesselnessMeasureImageFilter()
{
  m_MinScale = 0.77;
  m_MaxScale = 3.09375;
  m_ScaleMode = LINEAR;
  m_NumberOfSteps = 10;

  m_HessianFilter              = HessianFilterType::New();
  m_FAFilter                   = FAFilterType::New();
  m_FAFilter->SetUseDiffusion( true );

  //Instantiate Update buffer
  m_UpdateBuffer                 = UpdateBufferType::New();
}
/* ---------------------------------------------------------------------*/
template<class TInputImage, class TOutputImage>
void
MultiScaleHessian3DToFAVesselnessMeasureImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  // Allocate the output
  this->GetOutput()->SetBufferedRegion(
        this->GetOutput()->GetRequestedRegion() );
  this->GetOutput()->Allocate();

  // Allocate the buffer
  AllocateUpdateBuffer();

  typename CastFilterType::Pointer caster = CastFilterType::New();
  caster->SetInput( this->GetInput() );
  this->m_HessianFilter->SetInput( caster->GetOutput() );
  this->m_HessianFilter->SetNormalizeAcrossScale( true );

  double sigma = m_MinScale;

  int scaleLevel = 1;

  while ( sigma <= m_MaxScale )
  {
    std::cout << "At sigma: " << sigma << std::endl;
    m_HessianFilter->SetSigma( sigma );

    m_FAFilter->SetInput ( m_HessianFilter->GetOutput() );
    m_FAFilter->Update();
    this->UpdateMaximumResponse();
    sigma  = this->ComputeSigmaValue( scaleLevel );
    scaleLevel++;
  }

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
/* ---------------------------------------------------------------------*/
template <typename TInputImage, typename TOutputImage >
double
MultiScaleHessian3DToFAVesselnessMeasureImageFilter
<TInputImage,TOutputImage>
::ComputeSigmaValue( int ScaleLevel )
{
  double stepSize = 0, sigma = 0;

  switch (m_ScaleMode)
  {
  case LINEAR:
  {
    stepSize = (m_MaxScale-m_MinScale) / m_NumberOfSteps;
    if( stepSize <= 1e-10 )
    {
      stepSize = 1e-10;
    }
    sigma =  m_MinScale + (stepSize * ScaleLevel);
    if (m_MinScale == m_MaxScale)
      sigma+=0.1;
    break;
  }
  case EXPONENTIAL:
  {
    stepSize =
        ( vcl_log( m_MaxScale )  - vcl_log( m_MinScale) ) / m_NumberOfSteps;
    if( stepSize <= 1e-10 )
    {
      stepSize = 1e-10;
    }
    sigma = ( vcl_exp( vcl_log (m_MinScale) + stepSize * ScaleLevel) );
    if (m_MinScale == m_MaxScale)
      sigma+=0.1;
    break;
  }
  default:
  {
    std::cerr << "Error: Unknown scale mode option for the vesselness filter" << std::endl;
    return (m_MaxScale+1.0);
  }
  }

  return sigma;
}
/* ---------------------------------------------------------------------*/
template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessian3DToFAVesselnessMeasureImageFilter
<TInputImage,TOutputImage>
::UpdateMaximumResponse()
{

  typedef typename FAFilterType::OutputImageType
      FAOutputImageType;
  ImageRegionIterator<UpdateBufferType>
      oit(m_UpdateBuffer,m_UpdateBuffer->GetLargestPossibleRegion());
  ImageRegionIterator<FAOutputImageType>
      it(m_FAFilter->GetOutput(),
         this->m_FAFilter->GetOutput()->GetLargestPossibleRegion());

  it.GoToBegin();
  oit.GoToBegin();
  while(!oit.IsAtEnd())
  {
    if( oit.Value() < it.Value() )
    {
      oit.Value() = it.Value();
    }
    ++oit;
    ++it;
  }
}
/* ---------------------------------------------------------------------*/
template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessian3DToFAVesselnessMeasureImageFilter<TInputImage,TOutputImage>
::AllocateUpdateBuffer()
{
  /* The update buffer looks just like the output and holds the best response */

  typename TOutputImage::Pointer output = this->GetOutput();

  m_UpdateBuffer->SetSpacing(output->GetSpacing());
  m_UpdateBuffer->SetOrigin(output->GetOrigin());
  m_UpdateBuffer->SetLargestPossibleRegion(output->GetLargestPossibleRegion());
  m_UpdateBuffer->SetRequestedRegion(output->GetRequestedRegion());
  m_UpdateBuffer->SetBufferedRegion(output->GetBufferedRegion());
  m_UpdateBuffer->Allocate();
}

/* ---------------------------------------------------------------------
   PrintSelf method
   --------------------------------------------------------------------- */
template <class TInputImage, class TOutputImage>
void
MultiScaleHessian3DToFAVesselnessMeasureImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

} //end namespace

#endif //ITKMULTISCALEHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_TXX
