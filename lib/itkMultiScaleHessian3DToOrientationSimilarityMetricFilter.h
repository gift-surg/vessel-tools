#ifndef ITKMULTISCALEHESSIANTOORIENTATIONSIMILARITYMETRICFILTER_H
#define ITKMULTISCALEHESSIANTOORIENTATIONSIMILARITYMETRICFILTER_H

#include <itkImageToImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkHessianRecursiveGaussianImageFilter.h>

#include "itkHessian3DToOrientationSimilarityMetricFilter.h"

namespace itk {

/** \class MultiScaleHessian3DToOrientationSimilarityMetricFilter
 * \brief Given two images, it provides a measurement on how similar
 *  they are in terms of orientation using multiscale analysis
 */
template < class TInputImage, class TOutputImage >
class ITK_EXPORT
    MultiScaleHessian3DToOrientationSimilarityMetricFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef MultiScaleHessian3DToOrientationSimilarityMetricFilter  Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>  Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;
  typedef TInputImage                                    InputImageType;
  typedef TOutputImage                                   OutputImageType;

  /** Inherit types from Superclass. */
  typedef typename TInputImage::PixelType                InputPixelType;
  typedef typename TOutputImage::PixelType               OutputPixelType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiScaleHessian3DToOrientationSimilarityMetricFilter,
               ImageToImageFilter);

  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  void SetImageOne(const TInputImage* image);
  void SetImageTwo(const TInputImage* image);

  typedef enum
  {
    LINEAR = 0,
    EXPONENTIAL = 1
  } ScaleModeType;

  itkSetMacro(MinScale, float);
  itkGetConstMacro(MinScale, float);

  itkSetMacro(MaxScale, float);
  itkGetConstMacro(MaxScale, float);

  itkGetConstMacro(ScaleMode, ScaleModeType);
  itkSetMacro(ScaleMode, ScaleModeType);

  /** Set/Get macros for Number of Scales */
  itkSetMacro(NumberOfSteps, int);
  itkGetMacro(NumberOfSteps, int);

protected:
  MultiScaleHessian3DToOrientationSimilarityMetricFilter();

  ~MultiScaleHessian3DToOrientationSimilarityMetricFilter() { }
  void PrintSelf(std::ostream&os, Indent indent) const;

  typedef double InternalPixelType;
  typedef Image<InternalPixelType,ImageDimension> InternalImageType;
  typedef CastImageFilter< InputImageType, InternalImageType >
                                                         CastFilterType;
  typedef HessianRecursiveGaussianImageFilter< InternalImageType >
                                                         HessianFilterType;
  typedef Hessian3DToOrientationSimilarityMetricFilter< InternalPixelType >
                                                         OrientationFilterType;
  typedef Image< InternalPixelType, 3>                   UpdateBufferType;

  /** Generate the output data. */
  virtual void GenerateData();

private:
  MultiScaleHessian3DToOrientationSimilarityMetricFilter(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

  void UpdateMaximumResponse();
  double ComputeSigmaValue( int scaleLevel );
  void   AllocateUpdateBuffer();

  float                                       m_MinScale;
  float                                       m_MaxScale;
  ScaleModeType                               m_ScaleMode;
  int                                         m_NumberOfSteps;
  typename UpdateBufferType::Pointer          m_UpdateBuffer;
  typename OrientationFilterType::Pointer     m_OrientationFilter;
  typename HessianFilterType::Pointer         m_HessianFilterOne;
  typename HessianFilterType::Pointer         m_HessianFilterTwo;

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiScaleHessian3DToOrientationSimilarityMetricFilter.txx"
#endif
#endif // ITKORIENTATIONSIMILARITYMETRICFILTER_H
