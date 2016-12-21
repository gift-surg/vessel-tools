#ifndef ITKMULTISCALEHESSIAN3DTOFAORIENTATIONVESSELNESSMEASUREIMAGEFILTER_TXX
#define ITKMULTISCALEHESSIAN3DTOFAORIENTATIONVESSELNESSMEASUREIMAGEFILTER_TXX

#include "itkMultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter.h"

namespace itk {

template <typename TInputImage, typename TOutputImage >
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter<TInputImage,TOutputImage>
::MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter()
{
  m_MinScale = 0.77;
  m_MaxScale = 3.09375;
  m_ScaleMode = LINEAR;
  m_NumberOfSteps = 10;

  m_HessianFilterOne              = HessianFilterType::New();
  m_FAFilterOne                   = FAFilterType::New();
  m_FAFilterOne->SetUseDiffusion( true );

  m_HessianFilterTwo              = HessianFilterType::New();
  m_FAFilterTwo                   = FAFilterType::New();
  m_FAFilterTwo->SetUseDiffusion( true );

  m_OrientationFilter             = OrientationFilterType::New();
  m_OrientationFilter->SetDirectionIndex(1);

  //Instantiate Update buffer
  m_UpdateBuffer                 = UpdateBufferType::New();
}

template< class TInputImage, class TOutputImage >
void
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter<TInputImage, TOutputImage>
::SetImageOne(const TInputImage* image)
{
  this->SetNthInput(0, const_cast<TInputImage*>(image));
}

template< class TInputImage, class TOutputImage >
void
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter<TInputImage, TOutputImage>
::SetImageTwo(const TInputImage* image)
{
  this->SetNthInput(1, const_cast<TInputImage*>(image));
}


template<class TInputImage, class TOutputImage>
void
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  // Allocate the output
  this->GetOutput()->SetBufferedRegion(
                 this->GetOutput()->GetRequestedRegion() );
  this->GetOutput()->Allocate();

  // Allocate the buffer
  AllocateUpdateBuffer();

  typename CastFilterType::Pointer caster_one = CastFilterType::New();
  typename CastFilterType::Pointer caster_two = CastFilterType::New();
  caster_one->SetInput( static_cast< const InputImageType * >
                        ( this->ProcessObject::GetInput(0) ) );
  caster_two->SetInput( static_cast< const InputImageType * >
                        ( this->ProcessObject::GetInput(1) ) );
  this->m_HessianFilterOne->SetInput( caster_one->GetOutput() );
  this->m_HessianFilterOne->SetNormalizeAcrossScale( true );
  this->m_HessianFilterTwo->SetInput( caster_two->GetOutput() );
  this->m_HessianFilterTwo->SetNormalizeAcrossScale( true );

  double sigma = m_MinScale;
  int scaleLevel = 1;

  m_OrientationFilter->SetDirectionIndex(1);  //At some pointe remove invertibility
  while ( sigma <= m_MaxScale )
  {
    std::cout << "At sigma: " << sigma << std::endl;
    m_HessianFilterOne->SetSigma( sigma );
    m_HessianFilterTwo->SetSigma( sigma );

    m_FAFilterOne->SetInput(m_HessianFilterOne->GetOutput());
    m_FAFilterTwo->SetInput(m_HessianFilterTwo->GetOutput());
    m_FAFilterOne->Update();
    m_FAFilterTwo->Update();
    m_OrientationFilter->SetImageOne ( m_HessianFilterOne->GetOutput() );
    m_OrientationFilter->SetImageTwo( m_HessianFilterTwo->GetOutput() );
    m_OrientationFilter->Update();
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


template <typename TInputImage, typename TOutputImage >
double
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter
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

template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter
<TInputImage,TOutputImage>
::UpdateMaximumResponse()
{

  typedef typename FAFilterType::OutputImageType
                                         FAOutputImageType;
  typedef typename OrientationFilterType::OutputImageType
                                         OrientationOutputImageType;
  ImageRegionIterator<UpdateBufferType>
            oit(m_UpdateBuffer,m_UpdateBuffer->GetLargestPossibleRegion());
  ImageRegionIterator<FAOutputImageType>
            itOne(m_FAFilterOne->GetOutput(),
            this->m_FAFilterOne->GetOutput()->GetLargestPossibleRegion());
  ImageRegionIterator<FAOutputImageType>
            itTwo(m_FAFilterTwo->GetOutput(),
            this->m_FAFilterTwo->GetOutput()->GetLargestPossibleRegion());
  ImageRegionIterator<OrientationOutputImageType>
            it(m_OrientationFilter->GetOutput(),
            this->m_OrientationFilter->GetOutput()->GetLargestPossibleRegion());

  it.GoToBegin();
  itOne.GoToBegin();
  itTwo.GoToBegin();
  oit.GoToBegin();
  while(!oit.IsAtEnd())
  {
    OutputPixelType outVal = 0.5 * it.Value() * (itOne.Value()+itTwo.Value());
    if( oit.Value() < outVal )
    {
      oit.Value() = outVal;
    }
    ++oit;
    ++it;
    ++itOne;
    ++itTwo;
  }
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter<TInputImage,TOutputImage>
::AllocateUpdateBuffer()
{
  /* The update buffer looks just like the output and holds the best response
     in the measure */

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
MultiScaleHessian3DToFAOrientationVesselnessMeasureImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

}


#endif
