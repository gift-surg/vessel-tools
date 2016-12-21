#ifndef ITKHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_H
#define ITKHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_H

#include <itkImageToImageFilter.h>
#include <itkSymmetricSecondRankTensor.h>
#include <itkSymmetricEigenAnalysisImageFilter.h>

#define K  1.22474487139
namespace itk {

/** \class Hessian3DToFAVesselnessMeasureImageFilter
 * \brief Provides a vesselness measurement using fractional anisotropy
 */
template < typename  TPixel >
class ITK_EXPORT
Hessian3DToFAVesselnessMeasureImageFilter :
    public ImageToImageFilter< Image< SymmetricSecondRankTensor< double, 3 >, 3 >,
    Image< TPixel, 3 > >
{
public:
    /** Standard class typedefs. */
  typedef Hessian3DToFAVesselnessMeasureImageFilter Self;
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
  itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension);
  itkStaticConstMacro(InputPixelDimension, unsigned int,
                     InputPixelType::Dimension);

  /** Method for creation through the object factory. */
 itkNewMacro(Self);
 /** Run-time type information (and related methods). */
 itkTypeMacro(Hessian3DToFAVesselnessMeasureImageFilter, ImageToImageFilter);


  itkGetConstMacro(UseDiffusion, bool);
  itkSetMacro(UseDiffusion, bool);
  itkBooleanMacro(UseDiffusion);
#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(DoubleConvertibleToOutputCheck,
                  (Concept::Convertible<double, OutputPixelType>));
  /** End concept checking */
#endif

protected:
  Hessian3DToFAVesselnessMeasureImageFilter();
  ~Hessian3DToFAVesselnessMeasureImageFilter() { }

  void PrintSelf(std::ostream&os, Indent indent) const;

  /** Generate the output data. */
  virtual void GenerateData();

  typedef  FixedArray< double, itkGetStaticConstMacro(ImageDimension) > EigenValueType;
  typedef  Matrix< double, itkGetStaticConstMacro(ImageDimension),
                               itkGetStaticConstMacro(ImageDimension) > EigenVectorType;
  typedef SymmetricEigenAnalysis< EigenVectorType, EigenValueType,
                                  EigenVectorType > EigenAnalysisType;

private:
  Hessian3DToFAVesselnessMeasureImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

  bool m_UseDiffusion;
  //const double K = 1.22474487139;
  //const double EPSILON = 1e-03;
};

} //end namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessian3DToFAVesselnessMeasureImageFilter.txx"
#endif
#endif // ITKHESSIAN3DTOFAVESSELNESSMEASUREIMAGEFILTER_H
