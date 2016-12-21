/*=========================================================================

Seniha Esen Yuksel 06/09/2006
University of Florida

This is to implement the "multiscale vessel enhancement filtering" paper by
A.F. Frangi, W.J. Niessen, K.L.Vincken, M.A. Viergever
MICCAI 1998, pp. 130-137.

=========================================================================*/
#ifndef _itkHessian2DToVesselnessMeasureImageFilter_cxx
#define _itkHessian2DToVesselnessMeasureImageFilter_cxx

#include "itkHessian2DToVesselnessMeasureImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "vnl/vnl_math.h"

namespace itk
{

  //=======Constructor========
  template < typename TPixel >
  Hessian2DToVesselnessMeasureImageFilter< TPixel >::Hessian2DToVesselnessMeasureImageFilter()
  {
        // Hessian( Image ) = Jacobian( Gradient ( Image ) )  is symmetric
        m_SymmetricEigenValueFilter = EigenAnalysisFilterType::New();
        m_SymmetricEigenValueFilter->SetDimension( ImageDimension );
        m_SymmetricEigenValueFilter->OrderEigenValuesBy(
        EigenAnalysisFilterType::FunctorType::OrderByValue ); // will do lambda_1 < lambda_2 < ...

        m_EigenMagnitudeFilter = EigenMagnitudeFilterType::New();
        m_EigenMagnitudeFilter->SetDimension( ImageDimension );
        m_EigenMagnitudeFilter->OrderEigenValuesBy(
        EigenMagnitudeFilterType::FunctorType::OrderByMagnitude ); // will do |lambda_1| < |lambda_2| < ...
  }


  template < typename TPixel >
  void Hessian2DToVesselnessMeasureImageFilter< TPixel > ::GenerateData()
  {
        itkDebugMacro(<< "Hessian2DToVesselnessMeasureImageFilter generating data ");
        m_SymmetricEigenValueFilter->SetInput( this->GetInput() );
        m_EigenMagnitudeFilter->SetInput( this->GetInput() );

        typename OutputImageType::Pointer output = this->GetOutput();
        typedef typename EigenAnalysisFilterType::OutputImageType EigenValueImageType;

        m_SymmetricEigenValueFilter->Update();
        const typename EigenValueImageType::ConstPointer eigenImage = m_SymmetricEigenValueFilter->GetOutput();

        //================

        typedef typename EigenMagnitudeFilterType::OutputImageType  EigenMagnitudeImageType;
        m_EigenMagnitudeFilter->Update();
        const typename EigenMagnitudeImageType::ConstPointer eigenMagnitudeImage = m_EigenMagnitudeFilter->GetOutput();

        //iterators to walk the region of eigen values
        EigenValueArrayType eigenValue;
        ImageRegionConstIterator<EigenValueImageType> it;
        it = ImageRegionConstIterator<EigenValueImageType>(
              eigenImage, eigenImage->GetRequestedRegion());
        ImageRegionIterator<OutputImageType> oit;
        this->AllocateOutputs();
        oit = ImageRegionIterator<OutputImageType>(output,
                                             output->GetRequestedRegion());

        // iterators to walk the region of eigen magnitudes
        EigenValueArrayType eigenMagnitude;
        ImageRegionConstIterator<EigenMagnitudeImageType> itMag;
        itMag = ImageRegionConstIterator<EigenMagnitudeImageType>(
        eigenMagnitudeImage, eigenMagnitudeImage->GetRequestedRegion());
        ImageRegionIterator<OutputImageType> oitMag;
        this->AllocateOutputs();

        //=====calculating the second order structuredness S for all the eigenvalues ========
        //===== this part is for the calculation of ce -- half of the max. Hessian norm.=====
        oit.GoToBegin();
        it.GoToBegin();
        double dummyS =0;
        double maxS=0;
        while (!it.IsAtEnd())
        {
           // Get the eigen value
           eigenValue = it.Get();
           dummyS = vcl_sqrt( (eigenValue[0] * eigenValue[0]) + (eigenValue[1] * eigenValue[1]));
           if (dummyS>maxS) maxS=dummyS;
           ++it;
           ++oit;
        }
        double ce = maxS/2;


        //======== calculate vesselness =================
        oit.GoToBegin();
        it.GoToBegin();
        itMag.GoToBegin();

        while (!it.IsAtEnd())
        {
            // Get the eigen value
            eigenValue = it.Get();
            eigenMagnitude = itMag.Get();

            //the paper assumes the eigenvalues are sorted by magnitude
            //but then uses the real values of the eigenvalues not the magnitudes.
            //this is to handle that.

            double dummy0 = eigenValue[0];
            double dummy1 = eigenValue[1];
            if (vnl_math_abs(eigenValue[0]) > vnl_math_abs(eigenValue[1]))
            {
              eigenValue[0] = dummy1;
              eigenValue[1] = dummy0;
            }

          /*  double dummy0 = eigenValue[0];
            double dummy1 = eigenValue[1];
            if (eigenMagnitude[0] == vnl_math_abs( eigenValue[0] ))
                {
                  eigenValue[0] =  vnl_math_sgn( eigenValue[0] ) * eigenMagnitude[0] ;
                  eigenValue[1] =  vnl_math_sgn( eigenValue[1] ) * eigenMagnitude[1] ;
                }
            else
                {
                  eigenValue[0] =  vnl_math_sgn( eigenValue[1] ) * eigenMagnitude[0] ;
                  eigenValue[1] =  vnl_math_sgn( dummy0 ) * eigenMagnitude[1] ;
                }*/
            //end of playing around the eigenvalues

            //======== calculate vesselness =================

            double Rb = eigenValue[0]/eigenValue[1];
            double S=0;
            S = vcl_sqrt( ( eigenValue[0] * eigenValue[0]) + (eigenValue[1] *
                        eigenValue[1] ) );

            if(  eigenValue[1] <= 0 )
                {
                      double lineMeasure = 0;
                      double beta = 1;
                      lineMeasure = vcl_exp(-0.5 *Rb * Rb /( beta * beta) )
                   * (1 - vcl_exp(-0.5 *S * S /( ce * ce) ));
                      oit.Set( static_cast< OutputPixelType >(lineMeasure) );
                }
            else
                {
                      oit.Set( NumericTraits< OutputPixelType >::Zero );
                }

            ++it;
            ++oit;
            ++itMag;
    } //end of while

}

template < typename TPixel >
void
Hessian2DToVesselnessMeasureImageFilter< TPixel >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

}


} // end namespace itk

#endif
