#ifndef ITKMULTISCALETENSOR3DTOMAXLAMBDA_H
#define ITKMULTISCALETENSOR3DTOMAXLAMBDA_H

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkHessianEigenValueDecomposition.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"

namespace itk {
template <class TInputImage,
          class TOutputImage = TInputImage >
class ITK_EXPORT MultiScaleTensor3DToMaxLambda :
    public ImageToImageFilter< TInputImage,TOutputImage >
{
public:
  typedef MultiScaleTensor3DToMaxLambda Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>              Superclass;

  typedef SmartPointer<Self>                                      Pointer;
  typedef SmartPointer<const Self>                                ConstPointer;

  typedef TInputImage                          InputImageType;
  typedef typename InputImageType::PixelType   InputPixelType;
  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::PixelType  OutputPixelType;
  typedef typename OutputImageType::RegionType OutputRegionType;

  /** Image dimension = 3. */
  itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension);

  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiScaleTensor3DToMaxLambda, ImageToImageFilter);


  typedef  FixedArray< double, itkGetStaticConstMacro(ImageDimension) > EigenValueType;
  typedef  Matrix< double, itkGetStaticConstMacro(ImageDimension),
  itkGetStaticConstMacro(ImageDimension) > EigenVectorType;
  typedef Image< EigenVectorType, ImageDimension>  EigenVectorImageType;
  typedef Image< EigenValueType, ImageDimension>  EigenValueImageType;
  typedef typename EigenVectorImageType::Pointer EigenVectorPointer;
  typedef typename EigenValueImageType::Pointer EigenValuePointer;

  typedef enum
  {
    LINEAR = 0,
    EXPONENTIAL = 1
  } ScaleModeType;


  itkGetConstMacro(ScaleMode, ScaleModeType);
  itkSetMacro(ScaleMode, ScaleModeType);

  itkGetConstMacro(DirectionIndex, unsigned int);
  itkSetMacro(DirectionIndex, unsigned int);


  itkBooleanMacro( SquaredHessian );
  itkGetConstMacro(SquaredHessian,  bool);
  itkSetMacro(SquaredHessian, bool);

  itkSetMacro(MinScale, double);
  itkGetMacro(MinScale, double);

  /** Set/Get macros for Beta */
  itkSetMacro(MaxScale, double);
  itkGetMacro(MaxScale, double);

  /** Set/Get macros for Number of Scales */
  itkSetMacro(NumberOfSteps, int);
  itkGetMacro(NumberOfSteps, int);

  /** Set/Get macros for whether the scales are logarithmic or linear */


  EigenVectorPointer GetEigenVectorImage()
  {
    return m_EigenVectorImage;
  }

  EigenValuePointer GetEigenValueImage()
  {
    return m_EigenValueImage;
  }


protected:
  MultiScaleTensor3DToMaxLambda();
  ~MultiScaleTensor3DToMaxLambda(){ }

  void PrintSelf(std::ostream&os, Indent indent) const;

  /** Generate the output data. */
  virtual void GenerateData();

  typedef double InternalPixelType;
  typedef Image<InternalPixelType,ImageDimension> InternalImageType;
  typedef CastImageFilter< InputImageType, InternalImageType > CastFilterType;

  typedef HessianRecursiveGaussianImageFilter< InternalImageType > HessianFilterType;

  typedef HessianEigenValueDecomposition< double >
  DecompositionFilterType;
  /** Update image buffer that holds the best vesselness response */
  typedef Image< double, 3>                              UpdateBufferType;

private:

  MultiScaleTensor3DToMaxLambda(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

  bool                            m_SquaredHessian;
  unsigned int                    m_DirectionIndex;
  EigenValuePointer m_EigenValueImage;
  EigenVectorPointer m_EigenVectorImage;

  float                                       m_MinScale;
  float                                       m_MaxScale;
  ScaleModeType                               m_ScaleMode;
  int                                               m_NumberOfSteps;

  bool                                              m_IsSigmaStepLog;

  typename DecompositionFilterType::Pointer         m_VesselnessFilter;
  typename HessianFilterType::Pointer               m_HessianFilter;


  UpdateBufferType::Pointer                         m_UpdateBuffer;

  void UpdateMaximumResponse();

  double ComputeSigmaValue( int scaleLevel );

  void   AllocateUpdateBuffer();
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiScaleTensor3DToMaxLambda.txx"
#endif
#endif // ITKMULTISCALETENSOR3DTOMAXLAMBDA_H
