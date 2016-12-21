/*=========================================================================

Seniha Esen Yuksel 06/09/2006
University of Florida

This is to implement the "multiscale vessel enhancement filtering" paper by
A.F. Frangi, W.J. Niessen, K.L.Vincken, M.A. Viergever
MICCAI 1998, pp. 130-137.

=========================================================================*/
#ifndef __itkHessian2DToVesselnessMeasureImageFilter_h
#define __itkHessian2DToVesselnessMeasureImageFilter_h

#include "itkSymmetricSecondRankTensor.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"

namespace itk
{

template < typename  TPixel >
class ITK_EXPORT Hessian2DToVesselnessMeasureImageFilter : public
ImageToImageFilter< Image< SymmetricSecondRankTensor< double, 2 >, 2 >,
                        Image< TPixel, 2 > >
{
public:
  /** Standard class typedefs. */
  typedef Hessian2DToVesselnessMeasureImageFilter Self;
  typedef ImageToImageFilter<
            Image< SymmetricSecondRankTensor< double, 2 >, 2 >,
            Image< TPixel, 2 > > Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  typedef typename Superclass::InputImageType InputImageType;
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename InputImageType::PixelType InputPixelType;
  typedef TPixel OutputPixelType;

  /** Image dimension = 2. */
//  itkStaticConstMacro(ImageDimension, unsigned int,
  //                    ::itk::GetImageDimension<InputImageType>::ImageDimension);
itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension);
  typedef   itk::FixedArray< double, InputPixelType::Dimension >
                                                          EigenValueArrayType;
  typedef  itk::Image< EigenValueArrayType, InputImageType::ImageDimension >
                                                          EigenValueImageType;
  typedef   SymmetricEigenAnalysisImageFilter<
              InputImageType, EigenValueImageType > EigenAnalysisFilterType;

  typedef   SymmetricEigenAnalysisImageFilter<
              InputImageType, EigenValueImageType > EigenMagnitudeFilterType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);


protected:
  Hessian2DToVesselnessMeasureImageFilter();
  ~Hessian2DToVesselnessMeasureImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Generate Data */
  void GenerateData( void );

private:
  Hessian2DToVesselnessMeasureImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typename EigenAnalysisFilterType::Pointer m_SymmetricEigenValueFilter;
  typename EigenMagnitudeFilterType::Pointer m_EigenMagnitudeFilter;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessian2DToVesselnessMeasureImageFilter.txx"
#endif

#endif
