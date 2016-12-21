#ifndef ITKMULTISCALEHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_H
#define ITKMULTISCALEHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_H

#include <itkImageToImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkHessianRecursiveGaussianImageFilter.h>

#include "itkHessian3DToFAVesselnessMeasureImageFilter.h"

namespace itk {
/** \class MultiScaleHessian3DToFAVesselnessMeasureImageFilter
 * \brief FA metric for vesselness
 */
template < class TInputImage, class TOutputImage >
class ITK_EXPORT
    MultiScaleHessian3DToFAVesselnessMeasureImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef MultiScaleHessian3DToFAVesselnessMeasureImageFilter  Self;
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
  itkTypeMacro(MultiScaleHessian3DToFAVesselnessMeasureImageFilter,
               ImageToImageFilter);

  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

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
  MultiScaleHessian3DToFAVesselnessMeasureImageFilter();
  ~MultiScaleHessian3DToFAVesselnessMeasureImageFilter() { }

  void PrintSelf(std::ostream&os, Indent indent) const;

  typedef double InternalPixelType;
  typedef Image<InternalPixelType,ImageDimension> InternalImageType;
  typedef CastImageFilter< InputImageType, InternalImageType > CastFilterType;
  typedef HessianRecursiveGaussianImageFilter< InternalImageType > HessianFilterType;
  typedef Hessian3DToFAVesselnessMeasureImageFilter< InternalPixelType > FAFilterType;
  typedef Image< InternalPixelType, 3>                              UpdateBufferType;

  /** Generate the output data. */
  virtual void GenerateData();

private:
  MultiScaleHessian3DToFAVesselnessMeasureImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

  void UpdateMaximumResponse();
  double ComputeSigmaValue( int scaleLevel );
  void   AllocateUpdateBuffer();

  float                                       m_MinScale;
  float                                       m_MaxScale;
  ScaleModeType                               m_ScaleMode;
  int                                         m_NumberOfSteps;
  typename UpdateBufferType::Pointer          m_UpdateBuffer;
  typename FAFilterType::Pointer              m_FAFilter;
  typename HessianFilterType::Pointer         m_HessianFilter;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiScaleHessian3DToFAVesselnessMeasureImageFilter.txx"
#endif

#endif // ITKMULTISCALEHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_H
