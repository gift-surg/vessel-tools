#ifndef ITKSTRUCTURETENSORIMAGEFILTER_TXX
#define ITKSTRUCTURETENSORIMAGEFILTER_TXX

#include "itkStructureTensorImageFilter.h"
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>


namespace itk
{

template< class TInputImage, class TOutputImage >
StructureTensorImageFilter<TInputImage,TOutputImage>
::StructureTensorImageFilter( void )
{
  m_NormalizeAcrossScale = true;

  unsigned int imageDimensionMinus1 = static_cast<int>(ImageDimension)-1;
  if( ImageDimension > 1)
  {
    m_SmoothingFilters.resize(imageDimensionMinus1);
  }


  for( unsigned int i = 0; i < imageDimensionMinus1; i++ )
  {
    m_SmoothingFilters[ i ] = GaussianFilterType::New();
    m_SmoothingFilters[ i ]->SetOrder( GaussianFilterType::ZeroOrder );
    m_SmoothingFilters[ i ]->SetNormalizeAcrossScale( m_NormalizeAcrossScale );
    m_SmoothingFilters[ i ]->ReleaseDataFlagOn();
  }

  // Outer Gaussian smoothing filter
  m_TensorComponentSmoothingFilter = GaussianFilterType::New();
  m_TensorComponentSmoothingFilter->SetOrder( GaussianFilterType::ZeroOrder );
  m_TensorComponentSmoothingFilter->SetNormalizeAcrossScale( m_NormalizeAcrossScale );
  //m_TensorComponentSmoothingFilter->ReleaseDataFlagOn();

  m_DerivativeFilter = DerivativeFilterType::New();
  m_DerivativeFilter->SetOrder( DerivativeFilterType::FirstOrder );
  m_DerivativeFilter->SetNormalizeAcrossScale( m_NormalizeAcrossScale );
  m_DerivativeFilter->SetInput( this->GetInput() );

  if( ImageDimension > 1 )
  {
    m_SmoothingFilters[0]->SetInput( m_DerivativeFilter->GetOutput() );
  }

  for( unsigned int i = 1; i < imageDimensionMinus1; i++ )
  {
    m_SmoothingFilters[ i ]->SetInput( m_SmoothingFilters[i-1]->GetOutput() );
  }

  m_ImageAdaptor = OutputImageAdaptorType::New();

  this->SetSigma( 1.0 );
  this->SetSigmaOuter( 1.0 );

}

/**
 * Set value of Sigma
 */
template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage>
::SetSigma( RealType sigma )
{

  m_Sigma = sigma;
  for( unsigned int i = 0; i < ImageDimension - 1; i++ )
  {
    m_SmoothingFilters[ i ]->SetSigma( sigma );
  }
  m_DerivativeFilter->SetSigma( sigma );

  this->Modified();

}


/**
 * Set value of SigmaOuter
 */
template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage>
::SetSigmaOuter( RealType sigma )
{
  m_SigmaOuter = sigma;
  m_TensorComponentSmoothingFilter->SetSigma( sigma );
  this->Modified();
}

/**
 * Set Normalize Across Scale Space
 */
template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage>
::SetNormalizeAcrossScale( bool normalize )
{

  m_NormalizeAcrossScale = normalize;

  for( unsigned int i = 0; i<ImageDimension-1; i++ )
  {
    m_SmoothingFilters[ i ]->SetNormalizeAcrossScale( normalize );
  }
  m_DerivativeFilter->SetNormalizeAcrossScale( normalize );

  this->Modified();

}

template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage>
::GenerateInputRequestedRegion() throw(InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method. this should
  // copy the output requested region to the input requested region
  Superclass::GenerateInputRequestedRegion();

  // This filter needs all of the input
  typename
  StructureTensorImageFilter< TInputImage, TOutputImage >
      ::InputImagePointer image
      = const_cast< InputImageType * >( this->GetInput() );
  image->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
}

template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage>
::EnlargeOutputRequestedRegion(DataObject *output)
{
  TOutputImage *out = dynamic_cast< TOutputImage* >(output);

  if(out)
  {
    out->SetRequestedRegion( out->GetLargestPossibleRegion() );
  }
}

/**
 * Compute filter for Gaussian kernel
 */
template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage >
::GenerateData(  )
{
  // Create a process accumulator for tracking the progress of this
  // mini-pipeline
  //  ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
  //  progress->SetMiniPipelineFilter(this);

  //  // Compute the contribution of each filter to the total progress.
  //  const double weight = 1.0 / ( ImageDimension * ImageDimension );
  //  for( unsigned int i = 0; i<ImageDimension-1; i++ )
  //  {
  //    progress->RegisterInternalFilter( m_SmoothingFilters[i], weight );
  //  }
  //  progress->RegisterInternalFilter( m_DerivativeFilter, weight );
  //  progress->ResetProgress();

  const typename TInputImage::ConstPointer   inputImage( this->GetInput() );

  m_ImageAdaptor->SetImage( this->GetOutput() );
  m_ImageAdaptor->SetLargestPossibleRegion(
        inputImage->GetLargestPossibleRegion() );
  m_ImageAdaptor->SetBufferedRegion( inputImage->GetBufferedRegion() );
  m_ImageAdaptor->SetRequestedRegion( inputImage->GetRequestedRegion() );
  m_ImageAdaptor->Allocate();

  m_DerivativeFilter->SetInput( inputImage );

  unsigned int imageDimensionMinus1 = static_cast<int>(ImageDimension)-1;
  for( unsigned int dim=0; dim < ImageDimension; dim++ )
  {
    unsigned int i=0;
    unsigned int j=0;
    while( i< imageDimensionMinus1 )
    {
      if( i == dim )
      {
        j++;
      }
      m_SmoothingFilters[ i ]->SetDirection( j );
      i++;
      j++;
    }
    m_DerivativeFilter->SetDirection( dim );

   typename GaussianFilterType::Pointer lastFilter;
    if( ImageDimension > 1 )
    {
      int imageDimensionMinus2 = static_cast<int>(ImageDimension)-2;
      lastFilter = m_SmoothingFilters[imageDimensionMinus2];
      lastFilter->Update();
    }
    else
    {
      m_DerivativeFilter->Update();
    }

    //    progress->ResetFilterProgressAndKeepAccumulatedProgress();

    // Copy the results to the corresponding component
    // on the output image of vectors
    m_ImageAdaptor->SelectNthElement( dim );

    typename InternalImageType::Pointer derivativeImage = lastFilter->GetOutput();

    ImageRegionIteratorWithIndex< InternalImageType > it(
          derivativeImage,
          derivativeImage->GetRequestedRegion() );

    ImageRegionIteratorWithIndex< OutputImageAdaptorType > ot(
          m_ImageAdaptor,
          m_ImageAdaptor->GetRequestedRegion() );

    const RealType spacing = inputImage->GetSpacing()[ dim ];

    it.GoToBegin();
    ot.GoToBegin();
    while( !it.IsAtEnd() )
    {
      ot.Set( it.Get() / spacing );
      ++it;
      ++ot;
    }

  }

  //Calculate the outer (diadic) product of the gradient.
  ImageRegionIteratorWithIndex< OutputImageType > ottensor(
        this->GetOutput(),
        this->GetOutput()->GetRequestedRegion() );

  ImageRegionIteratorWithIndex< OutputImageType > itgradient(
        this->GetOutput(),
        this->GetOutput()->GetRequestedRegion() );

  const unsigned int numberTensorElements
      = (ImageDimension*(ImageDimension+1))/2;
  std::vector<InternalPixelType> tmp( numberTensorElements );

  ottensor.GoToBegin();
  itgradient.GoToBegin();
  while( !itgradient.IsAtEnd() )
  {
    unsigned int count = 0;
    for( unsigned int j = 0; j < ImageDimension; ++j)
    {
      for(int k = j; k < ImageDimension; ++k)
      {
        tmp[count++] = itgradient.Get()[j]*itgradient.Get()[k];
      }
    }
    for(unsigned int j = 0; j < numberTensorElements; ++j)
    {
      ottensor.Value()[j] = tmp[j];
    }

    ++itgradient;
    ++ottensor;
  }

  //Finally, smooth the outer product components


  for(unsigned int i =0; i < numberTensorElements; i++)
  {
    typename DerivativeImageType::Pointer derImage = DerivativeImageType::New();
    derImage->SetLargestPossibleRegion( inputImage->GetLargestPossibleRegion() );
    derImage->SetBufferedRegion( inputImage->GetBufferedRegion() );
    derImage->SetRequestedRegion( inputImage->GetRequestedRegion() );
    derImage->Allocate();

    ImageRegionIteratorWithIndex< DerivativeImageType >
        derit(
          derImage,
          derImage->GetRequestedRegion() );

    ottensor.GoToBegin();
    derit.GoToBegin();
    while( !derit.IsAtEnd() )
    {
      derit.Value() = ottensor.Get()[i];
      ++derit;
      ++ottensor;
    }

    m_TensorComponentSmoothingFilter->SetInput(derImage);
    m_TensorComponentSmoothingFilter->Update();

    ImageRegionIteratorWithIndex< DerivativeImageType >
        smoothedIt( m_TensorComponentSmoothingFilter->GetOutput(),
                    m_TensorComponentSmoothingFilter->GetOutput()->GetRequestedRegion());

    ottensor.GoToBegin();
    smoothedIt.GoToBegin();

    while( !ottensor.IsAtEnd() )
    {
      ottensor.Value()[i] = smoothedIt.Get();
      ++smoothedIt;
      ++ottensor;
    }
  }

  ottensor.GoToBegin();
  EigenMatrixType tmpMatrix;
  m_EigenMatrixImage = EigenMatrixImageType::New();
  m_EigenMatrixImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  m_EigenMatrixImage->CopyInformation( this->GetInput() );
  m_EigenMatrixImage->Allocate();
  EigenIteratorType eit;
  eit = EigenIteratorType( m_EigenMatrixImage,
                           m_EigenMatrixImage->GetLargestPossibleRegion() );

  while( !ottensor.IsAtEnd() )
  {
    SymmetricSecondRankTensor<double, 3> tmpTensor = ottensor.Get();
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
    eit.Set( tmpMatrix );
    ++eit;
    ++ottensor;
  }

  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<
      InputImageType, VesselImageType >               FilterType;
   typename  FilterType::Pointer filter = FilterType::New();
    filter->SetInput(this->GetInput() );
    filter->SetSigma( m_Sigma );
    filter->Update();
    m_GradientMagnitudeImage = filter->GetOutput();
}


template< class TInputImage, class TOutputImage >
void
StructureTensorImageFilter<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << "NormalizeAcrossScale: " << m_NormalizeAcrossScale << std::endl;
  os << "Sigma: " << m_Sigma << std::endl;
  os << "SigmaOuter: " << m_SigmaOuter << std::endl;
}


} // End namespace itk

#endif // ITKSTRUCTURETENSORIMAGEFILTER_TXX
