#ifndef ITKHESSIANEIGENVALUEDECOMPOSITION_TXX
#define ITKHESSIANEIGENVALUEDECOMPOSITION_TXX

#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include "itkHessianEigenValueDecomposition.h"
#include <vnl/vnl_math.h>
#define EPSILON 1e-3

namespace itk {
template< typename TPixel >
HessianEigenValueDecomposition< TPixel >::HessianEigenValueDecomposition()
{
}

template< typename TPixel >
void
HessianEigenValueDecomposition< TPixel >::GenerateData()
{
  typename OutputImageType::Pointer output = this->GetOutput();
  ImageRegionIterator<OutputImageType> oit;
  this->AllocateOutputs();

  m_EigenVectorImage = EigenVectorImageType::New();
  m_EigenVectorImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  m_EigenVectorImage->CopyInformation( this->GetInput() );
  m_EigenVectorImage->Allocate();

  m_EigenValueImage = EigenValueImageType::New();
  m_EigenValueImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  m_EigenValueImage->CopyInformation( this->GetInput() );
  m_EigenValueImage->Allocate();

  oit = ImageRegionIterator<OutputImageType>(output,
                                             output->GetRequestedRegion());
  ImageRegionConstIterator<InputImageType> it;
  it = ImageRegionConstIterator<InputImageType>(
      this->GetInput(), this->GetInput()->GetRequestedRegion());

  ImageRegionIterator<EigenVectorImageType> evit;
  evit = ImageRegionIterator<EigenVectorImageType>(m_EigenVectorImage,
                                                   m_EigenVectorImage->GetRequestedRegion());

  ImageRegionIterator<EigenValueImageType> Lit;
  Lit = ImageRegionIterator<EigenValueImageType>(m_EigenValueImage,
                                                   m_EigenValueImage->GetRequestedRegion());

  //Initialise analyser
  EigenAnalysisType  eig;
  eig.SetDimension( ImageDimension );
  EigenVectorType eigenMatrix, tmpMatrix,orderVectors,tmpinvMatrix;
  eigenMatrix.Fill(0);

  EigenValueType eigenVal,orderValues;
  eigenVal.Fill(0);

  oit.GoToBegin();
  it.GoToBegin();
  evit.GoToBegin();
  Lit.GoToBegin();
  while (!oit.IsAtEnd())
  {
    bool failed_1 = false;
    SymmetricSecondRankTensor<double, 3> tmpTensor = it.Get();
    //ImgOne values
    tmpMatrix[0][0] = tmpTensor[0];
    tmpMatrix[0][1] = tmpTensor[1];
    tmpMatrix[0][2] = tmpTensor[2];
    tmpMatrix[1][0] = tmpTensor[1];
    tmpMatrix[1][1] = tmpTensor[3];
    tmpMatrix[1][2] = tmpTensor[4];
    tmpMatrix[2][0] = tmpTensor[2];
    tmpMatrix[2][1] = tmpTensor[4];
    tmpMatrix[2][2] = tmpTensor[5];

    eig.ComputeEigenValuesAndVectors( tmpMatrix, eigenVal, eigenMatrix );
    //Order eigenvalues by hand to be sure it works
    unsigned int index1, index2, index3;

    this->OrderEigenValuesByMagnitude(eigenVal,index1,index2,index3);

    if ( eigenVal[index2] >= 0.0 ||  eigenVal[index3] >= 0.0 ||
             vnl_math_abs( eigenVal[index2] ) < EPSILON  ||
             vnl_math_abs( eigenVal[index3] ) < EPSILON)
    {
      orderVectors.Fill(0);
      orderValues.Fill(0);
      evit.Set( orderVectors );
      Lit.Set(orderValues);
      oit.Set( NumericTraits< OutputPixelType >::Zero );
    }
    else
    {
      if (m_SquaredHessian)
      {
        tmpMatrix[0][0] *= tmpMatrix[0][0];
        tmpMatrix[0][1] *= tmpMatrix[0][1];
        tmpMatrix[0][2] *= tmpMatrix[0][2];
        tmpMatrix[1][0] *= tmpMatrix[1][0];
        tmpMatrix[1][1] *= tmpMatrix[1][1];
        tmpMatrix[1][2] *= tmpMatrix[1][2];
        tmpMatrix[2][0] *= tmpMatrix[2][0];
        tmpMatrix[2][1] *= tmpMatrix[2][1];
        tmpMatrix[2][2] *= tmpMatrix[2][2];
      }
      if (m_DirectionIndex == 3)
      {
        try
        {
          vnl_matrix_fixed< double, 3, 3 > tmpinvvnlmatrix = tmpMatrix.GetInverse();
          tmpinvMatrix = -tmpinvvnlmatrix;
          eig.ComputeEigenValuesAndVectors( tmpinvMatrix, eigenVal, eigenMatrix );
          this->OrderEigenValuesByMagnitude(eigenVal,index1,index2,index3);
        }
        catch(ExceptionObject e)
        {
          failed_1 = true;
          std::cerr << e.GetDescription() << std::endl;
          std::cout << " Caught an error perhaps due to the determinant" << std::endl;
          eig.ComputeEigenValuesAndVectors( tmpMatrix, eigenVal, eigenMatrix );
          std::cout << " Eigenvalues where: " << eigenVal[0] << "  " <<
                        eigenVal[1] << "  " << eigenVal[2] << "  " << std::endl;
        }
      }
      else if (m_SquaredHessian)
      {
          eig.ComputeEigenValuesAndVectors( tmpMatrix, eigenVal, eigenMatrix );
          this->OrderEigenValuesByMagnitude(eigenVal,index1,index2,index3);
      }

      orderValues[0] = eigenVal[index1];
      orderValues[1] = eigenVal[index2];
      orderValues[2] = eigenVal[index3];

//      std::cout << " Eigenvalues where: " << orderValues[0] << "  " <<
//                    orderValues[1] << "  " << orderValues[2] << "  " << std::endl;



      orderVectors[0][0] = eigenMatrix[index1][0];
      orderVectors[0][1] = eigenMatrix[index1][1];
      orderVectors[0][2] = eigenMatrix[index1][2];
      orderVectors[1][0] = eigenMatrix[index2][0];
      orderVectors[1][1] = eigenMatrix[index2][1];
      orderVectors[1][2] = eigenMatrix[index2][2];
      orderVectors[2][0] = eigenMatrix[index3][0];
      orderVectors[2][1] = eigenMatrix[index3][1];
      orderVectors[2][2] = eigenMatrix[index3][2];

      evit.Set( orderVectors );
      Lit.Set(orderValues);
      if (m_DirectionIndex ==3) // diffusion thingy
        oit.Set(eigenVal[index3]);
      else //hessian side
        oit.Set(eigenVal[index3]/eigenVal[index2]);

    }
    ++evit;
    ++Lit;
    ++oit;
    ++it;

  }
}

template< typename TPixel >
void
HessianEigenValueDecomposition< TPixel >
::OrderEigenValuesByMagnitude(EigenValueType eigenVal, unsigned int &indexone,
                                 unsigned int &indextwo, unsigned int &indexthree)
{
  double smallest = vnl_math_abs( eigenVal[0] );
  double Lambda1 = eigenVal[0];
  indexone = 0, indextwo = 0, indexthree = 0;

  for ( unsigned int i=1; i <=2; i++ )
  {
    if ( vnl_math_abs( eigenVal[i] ) < smallest )
    {
      Lambda1 = eigenVal[i];
      smallest = vnl_math_abs( eigenVal[i] );
      indexone = i;
    }
  }
//    // Find the largest eigenvalue
  double largest = vnl_math_abs( eigenVal[0] );
  double Lambda3 = eigenVal[0];

  for ( unsigned int i=1; i <=2; i++ )
  {
    if (  vnl_math_abs( eigenVal[i] ) > largest  )
    {
      Lambda3 = eigenVal[i];
      largest = vnl_math_abs( eigenVal[i] );
      indexthree = i;
    }
  }

  //  find Lambda2 so that |Lambda1| < |Lambda2| < |Lambda3|
  for ( unsigned int i=0; i <=2; i++ )
  {
    if ( eigenVal[i] != Lambda1 && eigenVal[i] != Lambda3 )
    {
     // Lambda2 = eigenVal[i];
      indextwo =i;
      break;
    }
  }
}

template< typename TPixel >
void
HessianEigenValueDecomposition< TPixel >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

}

} //end namespace

#endif
