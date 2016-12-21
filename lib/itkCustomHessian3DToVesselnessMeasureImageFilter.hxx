/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkCustomHessian3DToVesselnessMeasureImageFilter_hxx
#define __itkCustomHessian3DToVesselnessMeasureImageFilter_hxx

#include "itkCustomHessian3DToVesselnessMeasureImageFilter.h"
#include "itkImageRegionIterator.h"
#include "vnl/vnl_math.h"
#include <limits>

namespace itk
{
/**
 * Constructor
 */
template< typename TPixel >
CustomHessian3DToVesselnessMeasureImageFilter< TPixel >
::CustomHessian3DToVesselnessMeasureImageFilter()
{
  m_Alpha1 = 0.5;
  m_Alpha2 = 2.0;

  // Hessian( Image ) = Jacobian( Gradient ( Image ) )  is symmetric
  m_SymmetricEigenValueFilter = EigenAnalysisFilterType::New();
  m_SymmetricEigenValueFilter->SetDimension(ImageDimension);
  m_SymmetricEigenValueFilter->OrderEigenValuesBy(
        EigenAnalysisFilterType::FunctorType::OrderByValue);
}

template< typename TPixel >
void
CustomHessian3DToVesselnessMeasureImageFilter< TPixel >
::GenerateData()
{
  itkDebugMacro(<< "CustomHessian3DToVesselnessMeasureImageFilter generating data ");

  m_SymmetricEigenValueFilter->SetInput( this->GetInput() );

  typename OutputImageType::Pointer output = this->GetOutput();

  typedef typename EigenAnalysisFilterType::OutputImageType
      EigenValueOutputImageType;

  m_SymmetricEigenValueFilter->Update();

  const typename EigenValueOutputImageType::ConstPointer eigenImage =
      m_SymmetricEigenValueFilter->GetOutput();

  // walk the region of eigen values and get the vesselness measure
  EigenValueArrayType                                   eigenValue;
  ImageRegionConstIterator< EigenValueOutputImageType > it;
  it = ImageRegionConstIterator< EigenValueOutputImageType >(
        eigenImage, eigenImage->GetRequestedRegion() );
  ImageRegionIterator< OutputImageType > oit;
  this->AllocateOutputs();
  oit = ImageRegionIterator< OutputImageType >( output,
                                                output->GetRequestedRegion() );
  oit.GoToBegin();
  it.GoToBegin();
  while ( !it.IsAtEnd() )
  {
    // Get the eigen value
    eigenValue = it.Get();

    // normalizeValue <= 0 for bright line structures
    double normalizeValue = vnl_math_min(-1.0 * eigenValue[1], -1.0 * eigenValue[0]);

    // Similarity measure to a line structure
    if ( normalizeValue > 0 ) // First modification: Let this to be negative
    {
      double lineMeasure;
      if ( eigenValue[2] <= 0)
      {
        lineMeasure =
            vcl_exp( -0.5 * vnl_math_sqr( eigenValue[2] / ( m_Alpha1 * normalizeValue ) ) );
      }
      else
      {
        lineMeasure =
            vcl_exp( -0.5 * vnl_math_sqr( eigenValue[2] / ( m_Alpha2 * normalizeValue ) ) );
      }

      double min_val = NumericTraits< OutputPixelType >::max();
      double max_val = NumericTraits< OutputPixelType >::Zero;

//      for (unsigned int i = 0; i < 3; ++i)
//      {
//        if (min_val > abs(eigenValue[i]))
//          min_val = abs((double) eigenValue[i]);
//        if (max_val < abs(eigenValue[i]))
//          max_val = abs((double) eigenValue[i]);
//      }

//      if (abs(min_val - max_val) <= NumericTraits< OutputPixelType >::epsilon())
//      {
//          lineMeasure = NumericTraits< OutputPixelType >::One;
//      }
//      if (min_val ==  NumericTraits< OutputPixelType >::max() || max_val == NumericTraits< OutputPixelType >::Zero
//              || max_val <= NumericTraits< OutputPixelType >::epsilon())
//          lineMeasure = NumericTraits< OutputPixelType >::Zero;
//      else
//      {
//        lineMeasure = abs((double) min_val / (double) max_val);
//        //std::cout << min_val << " - " << max_val << std::endl;
//      }
    //  lineMeasure = (eigenValue[1] /eigenValue[2] + eigenValue[1] /eigenValue[3] +eigenValue[2] +eigenValue[3] )/3.0;
      lineMeasure *= normalizeValue;
      lineMeasure = abs(eigenValue[0])*(abs(eigenValue[2])-abs(eigenValue[1]));
      oit.Set( static_cast< OutputPixelType >( lineMeasure ) );
    }
    else
    {
      oit.Set(NumericTraits< OutputPixelType >::Zero);
    }

    ++it;
    ++oit;
  }
}

template< typename TPixel >
void
CustomHessian3DToVesselnessMeasureImageFilter< TPixel >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Alpha1: " << m_Alpha1 << std::endl;
  os << indent << "Alpha2: " << m_Alpha2 << std::endl;
}
} // end namespace itk

#endif
