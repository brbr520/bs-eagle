#ifndef _MATRIX_IMPL_H
#define _MATRIX_IMPL_H
/** 
 * @file matrix_impl.h
 * @brief block matrix implementation
 * @author Oleg Borschuk
 * @date 2009-08-11
 */

/** 
* @brief interface class for matrix storage and manipulation
*/
template <class fp_type_t, class i_type_t>
class matrix_impl
{
  //-----------------------------------------
  //  METHODS
  //-----------------------------------------
public:
  //! constructor
  matrix_impl ()
    {
      n_rows = n_cols = 0;
      n_block_size = 1;
    }

  //! destructor
  virtual ~matrix_impl ()
  {
  }

  //! return block size 
  virtual i_type_t get_n_block_size () const
  {
    return n_block_size;
  }

  //! return number of rows in matrix 
  virtual i_type_t get_n_rows () const 
  {
    return n_rows;
  }

  //! return number of cols in matrix (return -1 in number of columns is unknown)
  virtual i_type_t get_n_cols () const
  {
    return n_cols;
  }

  //! return true if number of rows is equal to the number of columns
  virtual bool is_square ()
    {
      return (n_rows && n_cols);
    }

  //-----------------------------------------
  //  VARIABLES
  //-----------------------------------------
protected:
  i_type_t n_block_size;
  i_type_t n_rows;
  i_type_t n_cols;
};

#endif //_MATRIX_IMPL_H


