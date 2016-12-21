#ifndef ITKMULTISCALEHESSIANSMOOTHED3DTOVESSELNESSMEASUREIMAGEVECTORFILTER_H
#define ITKMULTISCALEHESSIANSMOOTHED3DTOVESSELNESSMEASUREIMAGEVECTORFILTER_H

#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
#include "itkHessianSmoothed3DToVesselnessMeasureImageVectorFilter.h"


namespace itk
{
/**\class MultiScaleHessianSmoothed3DToVesselnessMeasureImageFilter
 * \brief A filter to enhance 3D vascular structures using Hessian
 *         eigensystem in a multiscale framework
 *
 * The vesselness measure is based on the analysis of the the Hessian
 * eigen system. The vesseleness function is a smoothed (continous)
 * version of the Frangi's vesselness function. The filter takes an
 * image of any pixel type and generates a Hessian image pixels at different
 * scale levels. The vesselness measure is computed from the Hessian image
 * at each scale level and the best response is selected.  The vesselness
 * measure is computed using HessianSmoothed3DToVesselnessMeasureImageFilter.
 *
 * Minimum and maximum sigma value can be set using SetMinSigma and
 * SetMaxSigma methods respectively. The number of scale levels is set
 * using SetNumberOfSigmaSteps method. Exponentially (or linear with
 * the Gerardus modification) distributed scale levels are computed
 * within the bound set by the minimum and maximum sigma values
 *
 *
 * \par References
 *  Manniesing, R, Viergever, MA, & Niessen, WJ (2006). Vessel Enhancing
 *  Diffusion: A Scale Space Representation of Vessel Structures. Medical
 *  Image Analysis, 10(6), 815-825.
 *
 * \sa MultiScaleHessianSmoothed3DToVesselnessMeasureImageFilter
 * \sa Hessian3DToVesselnessMeasureImageFilter
 * \sa HessianSmoothedRecursiveGaussianImageFilter
 * \sa SymmetricEigenAnalysisImageFilter
 * \sa SymmetricSecondRankTensor
 *
 * \ingroup IntensityImageFilters TensorObjects
 *
 */
template <class TInputImage,
          class TOutputImage = TInputImage >
class ITK_EXPORT MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter
: public
ImageToImageFilter< TInputImage,TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>              Superclass;

  typedef SmartPointer<Self>                                      Pointer;
  typedef SmartPointer<const Self>                                ConstPointer;


  typedef TInputImage                                    InputImageType;
  typedef TOutputImage                                   OutputImageType;

  typedef typename TInputImage::PixelType                InputPixelType;
  typedef typename TOutputImage::PixelType               OutputPixelType;

  /** Update image buffer that holds the best vesselness response */
  typedef Image< double, 3>                              UpdateBufferType;


  /** Image dimension = 3. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                   InputImageType::ImageDimension);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  // Define image of matrix pixel type
  typedef Matrix< double, ImageDimension, ImageDimension> EigenMatrixType;
  typedef Image< EigenMatrixType, ImageDimension>  EigenMatrixImageType;
  typedef typename EigenMatrixImageType::Pointer EigenMatrixPointer;


  typedef enum
  {
    LINEAR = 0,
    EXPONENTIAL = 1
  } ScaleModeType;

  itkGetConstMacro(ScaleMode, ScaleModeType);
  itkSetMacro(ScaleMode, ScaleModeType);

  /** Set/Get macros for Alpha */
  itkSetMacro(SigmaMin, double);
  itkGetMacro(SigmaMin, double);

  /** Set/Get macros for Beta */
  itkSetMacro(SigmaMax, double);
  itkGetMacro(SigmaMax, double);

  /** Set/Get macros for Number of Scales */
  itkSetMacro(NumberOfSigmaSteps, int);
  itkGetMacro(NumberOfSigmaSteps, int);

  /** Set/Get macros for whether the scales are logarithmic or linear */
  itkSetMacro(IsSigmaStepLog, bool);
  itkGetMacro(IsSigmaStepLog, bool);

  /** Set/Get macros for whether structures are bright or dark */
  itkSetMacro( BrightVessels, bool );
  itkGetMacro( BrightVessels, bool );

  itkSetMacro( ComputeMatrix, bool );
  itkGetMacro( ComputeMatrix, bool );

  EigenMatrixPointer GetEigenMatrix()
    {
    return m_EigenMatrixImage;
    }


protected:
  MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter();
  ~MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  typedef HessianRecursiveGaussianImageFilter< InputImageType >
                                                        HessianFilterType;

  typedef HessianSmoothed3DToVesselnessMeasureImageVectorFilter< double >
                                                        VesselnessFilterType;

  /** Generate Data */
  void GenerateData( void );

private:
  void UpdateMaximumResponse();

  double ComputeSigmaValue( int scaleLevel );

  void   AllocateUpdateBuffer();

  //purposely not implemented
  MultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter(const Self&);
  void operator=(const Self&); //purposely not implemented

  double                                            m_SigmaMin;
  double                                            m_SigmaMax;

  int                                               m_NumberOfSigmaSteps;
  ScaleModeType                                     m_ScaleMode;
  bool                                              m_IsSigmaStepLog;
  bool                                              m_BrightVessels;
  bool                                              m_ComputeMatrix;
  typename VesselnessFilterType::Pointer            m_VesselnessFilter;
  typename HessianFilterType::Pointer               m_HessianFilter;
  EigenMatrixPointer                                m_EigenMatrixImage;


  UpdateBufferType::Pointer                         m_UpdateBuffer;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiScaleHessianSmoothed3DToVesselnessMeasureImageVectorFilter.txx"
#endif

#endif
