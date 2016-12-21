#ifndef ITKHESSIANEIGENVALUEDECOMPOSITION_H
#define ITKHESSIANEIGENVALUEDECOMPOSITION_H

#include <itkImageToImageFilter.h>
#include <itkSymmetricSecondRankTensor.h>
namespace itk {
template < typename  TPixel >
class ITK_EXPORT
    HessianEigenValueDecomposition :
    public ImageToImageFilter< Image< SymmetricSecondRankTensor< double, 3 >, 3 >,
    Image< TPixel, 3 > >
{
public:
  typedef HessianEigenValueDecomposition Self;
  typedef ImageToImageFilter<
  Image< SymmetricSecondRankTensor< double, 3 >, 3 >,
  Image< TPixel, 3 > >                 Superclass;

  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  typedef typename Superclass::InputImageType            InputImageType;
  typedef typename Superclass::OutputImageType           OutputImageType;
  typedef typename InputImageType::PixelType             InputPixelType;
  typedef TPixel                                         OutputPixelType;

  itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension);
  itkStaticConstMacro(InputPixelDimension, unsigned int,
                      InputPixelType::Dimension);

  typedef  FixedArray< double, itkGetStaticConstMacro(ImageDimension) > EigenValueType;
  typedef  Matrix< double, itkGetStaticConstMacro(ImageDimension),
  itkGetStaticConstMacro(ImageDimension) > EigenVectorType;
  typedef Image< EigenVectorType, ImageDimension>  EigenVectorImageType;
  typedef Image< EigenValueType, ImageDimension>  EigenValueImageType;
  typedef typename EigenVectorImageType::Pointer EigenVectorPointer;
  typedef typename EigenValueImageType::Pointer EigenValuePointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  /** Run-time type information (and related methods). */
  itkTypeMacro(HessianEigenValueDecomposition, ImageToImageFilter);

  itkGetConstMacro(DirectionIndex, unsigned int);
  itkSetMacro(DirectionIndex, unsigned int);

  itkBooleanMacro( SquaredHessian );
  itkGetConstMacro(SquaredHessian,  bool);
  itkSetMacro(SquaredHessian, bool);

  EigenVectorPointer GetEigenVectorImage()
  {
    return m_EigenVectorImage;
  }

  EigenValuePointer GetEigenValueImage()
  {
    return m_EigenValueImage;
  }


#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(DoubleConvertibleToOutputCheck,
                  (Concept::Convertible<double, OutputPixelType>));
  /** End concept checking */
#endif

protected:
  HessianEigenValueDecomposition();
  ~HessianEigenValueDecomposition(){ }

  void PrintSelf(std::ostream&os, Indent indent) const;

  /** Generate the output data. */
  virtual void GenerateData();


  typedef SymmetricEigenAnalysis< EigenVectorType, EigenValueType,
  EigenVectorType > EigenAnalysisType;
private:

  HessianEigenValueDecomposition(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented
  bool m_SquaredHessian;
  unsigned int m_DirectionIndex;
  EigenValuePointer m_EigenValueImage;
  EigenVectorPointer m_EigenVectorImage;

  void OrderEigenValuesByMagnitude(EigenValueType values, unsigned int& indexone,
                                   unsigned int& indextwo, unsigned int& indexthree);
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianEigenValueDecomposition.txx"
#endif
#endif // ITKHESSIANEIGENVALUEDECOMPOSITION_H
