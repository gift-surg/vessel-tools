#ifndef ITKMULTISCALETENSOR3DTOMAXLAMBDA_TXX
#define ITKMULTISCALETENSOR3DTOMAXLAMBDA_TXX
#include "itkMultiScaleTensor3DToMaxLambda.h"
#include <vnl/vnl_math.h>
#include <itkImageRegionConstIterator.h>
#include<itkImageRegionIterator.h>
namespace itk {

template <typename TInputImage, typename TOutputImage >
MultiScaleTensor3DToMaxLambda< TInputImage,TOutputImage >
::MultiScaleTensor3DToMaxLambda()
{
  m_MinScale = 0.2;
  m_MaxScale = 2.0;

  m_NumberOfSteps = 10;
  m_ScaleMode = EXPONENTIAL;

  m_HessianFilter                = HessianFilterType::New();
  m_VesselnessFilter             = DecompositionFilterType::New();
  m_EigenVectorImage             = EigenVectorImageType::New();
  m_EigenValueImage              = EigenValueImageType::New();
  //Turn off vesselness measure scaling
  m_VesselnessFilter->SetDirectionIndex( 1 );
  m_VesselnessFilter->SetSquaredHessian( false );

  //Instantiate Update buffer
  m_UpdateBuffer                 = UpdateBufferType::New();
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleTensor3DToMaxLambda< TInputImage,TOutputImage >
::GenerateData()
{
  // Allocate the output
  this->GetOutput()->SetBufferedRegion( this->GetOutput()->GetRequestedRegion() );
  this->GetOutput()->Allocate();

  // Allocate the buffer
  AllocateUpdateBuffer();

  typename CastFilterType::Pointer caster = CastFilterType::New();
  caster->SetInput( this->GetInput() );
  this->m_HessianFilter->SetInput( caster->GetOutput() );
  this->m_HessianFilter->SetNormalizeAcrossScale( true );
  m_VesselnessFilter->SetSquaredHessian(m_SquaredHessian);
  m_VesselnessFilter->SetDirectionIndex(m_DirectionIndex);

  double sigma = m_MinScale;
  int scaleLevel = 1;
  while ( sigma <= m_MaxScale )
  {
    std::cout << "At scale: " << sigma << std::endl;
    m_HessianFilter->SetSigma( sigma );
    m_VesselnessFilter->SetInput ( m_HessianFilter->GetOutput() );
    m_VesselnessFilter->Update();

    this->UpdateMaximumResponse();
    sigma  = this->ComputeSigmaValue( scaleLevel );
    std::cout << "At scale: " << sigma << std::endl;
    scaleLevel++;
  }
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleTensor3DToMaxLambda
<TInputImage,TOutputImage>
::AllocateUpdateBuffer()
{
  EigenValueType l;
  EigenVectorType p;
  l.Fill(0);
  p.Fill( 0 );

  m_EigenVectorImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  m_EigenVectorImage->CopyInformation( this->GetInput() );
  m_EigenVectorImage->Allocate();
  m_EigenVectorImage->FillBuffer( p );

  m_EigenValueImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  m_EigenValueImage->CopyInformation( this->GetInput() );
  m_EigenValueImage->Allocate();
  m_EigenValueImage->FillBuffer( l );

  typename TOutputImage::Pointer output = this->GetOutput();

  m_UpdateBuffer->SetSpacing(output->GetSpacing());
  m_UpdateBuffer->SetOrigin(output->GetOrigin());
  m_UpdateBuffer->SetLargestPossibleRegion(output->GetLargestPossibleRegion());
  m_UpdateBuffer->SetRequestedRegion(output->GetRequestedRegion());
  m_UpdateBuffer->SetBufferedRegion(output->GetBufferedRegion());
  m_UpdateBuffer->Allocate();
}

//TODO change things when a max is really needed (may be in the class underneath)
template <typename TInputImage, typename TOutputImage >
void
MultiScaleTensor3DToMaxLambda
<TInputImage,TOutputImage>
::UpdateMaximumResponse()
{
  OutputRegionType region = this->GetOutput()->GetLargestPossibleRegion();

  ImageRegionIterator< OutputImageType > oit( this->GetOutput(), region );
  ImageRegionIterator<EigenValueImageType>
      olit( m_EigenValueImage, region );
  ImageRegionIterator<EigenVectorImageType>
      oeit( m_EigenVectorImage, region );

  ImageRegionIterator<OutputImageType>
      it( m_VesselnessFilter->GetOutput(), region );
  ImageRegionIterator<EigenValueImageType>
      lit(m_VesselnessFilter->GetEigenValueImage(), region );
  ImageRegionIterator<EigenVectorImageType>
      eit( m_VesselnessFilter->GetEigenVectorImage(), region );
  oit.GoToBegin();
  oeit.GoToBegin();
  olit.GoToBegin();
  it.GoToBegin();
  eit.GoToBegin();
  lit.GoToBegin();

  while( !oit.IsAtEnd() )
  {
    if( oit.Value() < static_cast< OutputPixelType >( vnl_math_abs(it.Value()) ) )
    {
      oit.Value() = static_cast< OutputPixelType >( it.Value() );
      olit.Set( lit.Get() );
      oeit.Set( eit.Get() );
    }
    ++oit;
    ++it;
    ++lit;
    ++olit;
    ++eit;
    ++oeit;
  }
}

template <typename TInputImage, typename TOutputImage >
double
MultiScaleTensor3DToMaxLambda
<TInputImage,TOutputImage>
::ComputeSigmaValue( int ScaleLevel )
{
  double stepSize = 0, sigma = 0;
  std::cout << sigma << std::endl;
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
      if (m_MaxScale == m_MinScale)
        sigma =m_MaxScale+1.0;
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
      if (m_MaxScale == m_MinScale)
        sigma =m_MaxScale+0.1;
      break;
    }
    default:
    {
      std::cerr << "Error: Unknown scale mode option for the vesselness filter" << std::endl;
      return (m_MaxScale+1.0);
    }
  }
  std::cout << "out " << sigma << std::endl;
  return sigma;
}

template <typename TInputImage, typename TOutputImage >
void
MultiScaleTensor3DToMaxLambda
<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

}

}



#endif //ITKMULTISCALETENSOR3DTOMAXLAMBDA_TXX
