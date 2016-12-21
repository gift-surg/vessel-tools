#ifndef HESSIANSMOOTHED3DTOVESSELNESSMEASUREIMAGEVECTORFILTER_H
#define HESSIANSMOOTHED3DTOVESSELNESSMEASUREIMAGEVECTORFILTER_H

#ifndef __itkHessianSmoothed3DToVesselnessMeasureImageFilter_h
#define __itkHessianSmoothed3DToVesselnessMeasureImageFilter_h

#include "itkSymmetricSecondRankTensor.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"


class HessianSmoothed3DToVesselnessMeasureImageVectorFilter : public ImageToImageFilter
{
public:
  HessianSmoothed3DToVesselnessMeasureImageVectorFilter();
};

#endif
namespace itk
{
/** \class HessianSmoothed3DToVesselnessMeasureImageVectorFilter
 * \brief A filter to enhance 3D vascular structures
 *
 * The vesselness measure is based on the analysis of the the Hessian
 * eigen system. The vesseleness function is a smoothed (continuous)
 * version of the Frang's vesselness function. The filter takes an
 * image of a Hessian pixels ( SymmetricSecondRankTensor pixels ) and
 * produces an enhanced image. The Hessian input image can be produced using
 * itkHessianSmoothedRecursiveGaussianImageFilter.
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

template < typename  TPixel >
class ITK_EXPORT HessianSmoothed3DToVesselnessMeasureImageVectorFilter
    : public ImageToImageFilter< Image< SymmetricSecondRankTensor< double, 3 >, 3 >,
    Image< TPixel, 3 > >
{
public:
  /** Standard class typedefs. */
  typedef HessianSmoothed3DToVesselnessMeasureImageVectorFilter Self;

  typedef ImageToImageFilter<
  Image< SymmetricSecondRankTensor< double, 3 >, 3 >,
  Image< TPixel, 3 > >                 Superclass;

  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  typedef typename Superclass::InputImageType            InputImageType;
  typedef typename Superclass::OutputImageType           OutputImageType;
  typedef typename InputImageType::PixelType             InputPixelType;
  typedef TPixel                                         OutputPixelType;

  /** Image dimension = 3. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      InputImageType::ImageDimension);

  itkStaticConstMacro(InputPixelDimension, unsigned int,
                      InputPixelType::Dimension);

  typedef  FixedArray< double, itkGetStaticConstMacro(InputPixelDimension) >
                                                              EigenValueType;
  typedef  Image< EigenValueType, itkGetStaticConstMacro(ImageDimension) >
                                                          EigenValueImageType;

  typedef Matrix< double, itkGetStaticConstMacro(InputPixelDimension),
                          itkGetStaticConstMacro(InputPixelDimension) > EigenMatrixType;
  typedef Image< EigenMatrixType, itkGetStaticConstMacro(InputPixelDimension)>
                                                                  EigenMatrixImageType;
  typedef typename EigenMatrixImageType::Pointer EigenMatrixPointer;

  typedef SymmetricEigenAnalysis< EigenMatrixType, EigenValueType,
                                        EigenMatrixType > EigenAnalysisFilterType;

  typedef ImageRegionIteratorWithIndex< EigenMatrixImageType > EigenIteratorType;


  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Set/Get macros for Alpha */
  itkSetMacro(Alpha, double);
  itkGetMacro(Alpha, double);

  /** Set/Get macros for Beta */
  itkSetMacro(Beta, double);
  itkGetMacro(Beta, double);

  /** Set/Get macros for Gamma */
  itkSetMacro(Gamma, double);
  itkGetMacro(Gamma, double);

  /** Set/Get macros for C */
  itkSetMacro(C, double);
  itkGetMacro(C, double);

  /** Macro to scale the vesselness measure with the
      largest eigenvalue or not */
  itkSetMacro( ScaleVesselnessMeasure, bool );
  itkGetMacro( ScaleVesselnessMeasure, bool );
  itkBooleanMacro(ScaleVesselnessMeasure);

  /** Macro to detect dark structures or not */
  itkSetMacro( BrightVessels, bool );
  itkGetMacro( BrightVessels, bool );
  itkBooleanMacro(BrightVessels);

  itkSetMacro( ComputeEigenVectors, bool );
  itkGetMacro( ComputeEigenVectors, bool );
  itkBooleanMacro( ComputeEigenVectors );

  EigenMatrixPointer GetEigenMatrix()
  {
    return m_EigenMatrixImage;
  }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(DoubleConvertibleToOutputCheck,
                  (Concept::Convertible<double, OutputPixelType>));
  /** End concept checking */
#endif

protected:
  HessianSmoothed3DToVesselnessMeasureImageVectorFilter();
  ~HessianSmoothed3DToVesselnessMeasureImageVectorFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Generate Data */
  void GenerateData( void );
  void OrderEigenValuesByMagnitude(EigenValueType values, unsigned int& indexone,
                                   unsigned int& indextwo, unsigned int& indexthree);

private:

  //purposely not implemented
  HessianSmoothed3DToVesselnessMeasureImageVectorFilter(const Self&);

  void operator=(const Self&); //purposely not implemented

  EigenMatrixPointer                            m_EigenMatrixImage;
  double                                        m_Alpha;
  double                                        m_Beta;
  double                                        m_Gamma;

  double                                        m_C;

  bool                                          m_ScaleVesselnessMeasure;
  bool                                          m_BrightVessels;
  bool                                          m_ComputeEigenVectors;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianSmoothed3DToVesselnessMeasureImageVectorFilter.txx"
#endif

#endif // HESSIANSMOOTHED3DTOVESSELNESSMEASUREIMAGEVECTORFILTER_H


