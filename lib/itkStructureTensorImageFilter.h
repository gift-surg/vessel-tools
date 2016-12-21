#ifndef ITKSTRUCTURETENSORIMAGEFILTER_H
#define ITKSTRUCTURETENSORIMAGEFILTER_H

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkNthElementImageAdaptor.h>
#include <itkPixelTraits.h>
#include <itkProgressAccumulator.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkSymmetricSecondRankTensor.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIterator.h>

namespace itk
{


/** \class StructureTensorImageFilter
 * \brief Computes the structure tensor of an image
 */
template< class TInputImage,
          class TOutputImage = Image< SymmetricSecondRankTensor<
            typename NumericTraits< typename TInputImage::PixelType >::RealType,
            TInputImage::ImageDimension >, TInputImage::ImageDimension > >
class StructureTensorImageFilter
  : public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef StructureTensorImageFilter     Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>    Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  itkNewMacro( Self );

  typedef TInputImage                                     InputImageType;
  typedef typename TInputImage::PixelType                 PixelType;
  typedef typename NumericTraits<PixelType>::RealType     RealType;
//  typedef typename InputImageType::Pointer               InputImagePointer;

  /** Image dimension. */
  itkStaticConstMacro( ImageDimension,
                      unsigned int,
                      TInputImage::ImageDimension );

  /** Type of the output image */
  typedef TOutputImage                                    OutputImageType;
  typedef typename OutputImageType::Pointer               OutputImagePointer;
  typedef typename OutputImageType::PixelType             OutputPixelType;
  typedef typename PixelTraits< OutputPixelType >::ValueType
      OutputComponentType;
  typedef itk::Image< double, ImageDimension > VesselImageType;
  typedef typename VesselImageType::Pointer               VesselImagePointer;

  typedef Matrix< double, itkGetStaticConstMacro(ImageDimension),
                            itkGetStaticConstMacro(ImageDimension) > EigenMatrixType;
  typedef Image< EigenMatrixType, itkGetStaticConstMacro(ImageDimension)>
                                                                  EigenMatrixImageType;
  typedef typename EigenMatrixImageType::Pointer EigenMatrixPointer;
  typedef ImageRegionIteratorWithIndex< EigenMatrixImageType > EigenIteratorType;


  /** Set Sigma value. Sigma is measured in the units of image spacing.  */
  void SetSigma( RealType sigma );
  void SetSigmaOuter( RealType rho);

  /** Define which normalization factor will be used for the Gaussian */
  void SetNormalizeAcrossScale( bool normalizeInScaleSpace );
  itkGetMacro( NormalizeAcrossScale, bool );

  //Sigma value for the Gaussian derivative filters
  itkGetMacro( Sigma, RealType );

  //Sigma value for the outer Gaussian smoothing filter
  itkGetMacro( SigmaOuter,   RealType );

  EigenMatrixPointer GetEigenMatrix()
  {
    return m_EigenMatrixImage;
  }

  VesselImagePointer GetGradient()
  {
    return m_GradientMagnitudeImage;
  }

protected:
  StructureTensorImageFilter( void );
  virtual ~StructureTensorImageFilter( void ) {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  typedef float                                           InternalPixelType;
  typedef Image< InternalPixelType, itkGetStaticConstMacro( ImageDimension ) >
                                                              InternalImageType;
  typedef RecursiveGaussianImageFilter< InternalImageType, InternalImageType >
                                                              GaussianFilterType;

  typedef RecursiveGaussianImageFilter< InputImageType, InternalImageType >
                                                              DerivativeFilterType;
  typedef NthElementImageAdaptor< TOutputImage, InternalPixelType >
                                                              OutputImageAdaptorType;
  typedef Image<InternalPixelType, ImageDimension>            DerivativeImageType;

  typedef typename GaussianFilterType::Pointer                GaussianFilterPointer;

  virtual void GenerateInputRequestedRegion( void ) throw(InvalidRequestedRegionError);


  /** Generate Data */
  void GenerateData( void );

  // Override since the filter produces the entire data set
  void EnlargeOutputRequestedRegion(DataObject *output);

private:
  StructureTensorImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  std::vector<GaussianFilterPointer>                          m_SmoothingFilters;
  typename DerivativeFilterType::Pointer                    m_DerivativeFilter;
  typename GaussianFilterType::Pointer                      m_TensorComponentSmoothingFilter;
  typename OutputImageAdaptorType::Pointer                  m_ImageAdaptor;
  VesselImagePointer                          m_GradientMagnitudeImage;
    EigenMatrixPointer                            m_EigenMatrixImage;
  /** Normalize the image across scale space */
  bool m_NormalizeAcrossScale;


  InternalPixelType      m_Sigma;
  InternalPixelType      m_SigmaOuter;

}; // End class StructureTensorImageFilter


} // End namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStructureTensorImageFilter.txx"
#endif

#endif // ITKSTRUCTURETENSORIMAGEFILTER_H
