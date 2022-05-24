/*
 * ClassTamplate.h
 *
 *  Created on: Sep 23, 2014
 *      Author: user
 */

#ifndef CLASSTAMPLATE_H_
#define CLASSTAMPLATE_H_

namespace ClassTampl {
//=============================================================================
template <class elemType>
class Tlist_item
{
public:

   Tlist_item( elemType &value );

   elemType     value();
   void         value(elemType &new_value);
   Tlist_item*  next();

   elemType     *ptr();

   void         next(Tlist_item *link );
   //void         prew(Tlist_item *link );
private:
   elemType _value;
   Tlist_item *_next;

};
//=============================================================================
template <class elemType>
class Tlist
{
public:

   Tlist();
   Tlist(const Tlist<elemType> &rhs);
   ~Tlist();
   elemType         operator[]( int ix ) const;
   Tlist<elemType> &operator=( const Tlist<elemType>& );

   int          size();
   bool         empty();
   //void         insert(Tlist_item<elemType> *ptr, elemType value);
   void         push_front(elemType value);
   void         push_back(elemType value);
   void         pop_front();
   void         pop_back();

   elemType*    front();
   elemType*    back();
   //void         erase( int index );

   int          remove( elemType value );
   void         clear ();
   void         concat( const Tlist<elemType> &i1 );
   void         reverse();

   Tlist_item<elemType>*  find(elemType value);

   elemType     next();
   elemType     foreach_next();
   void         set_interator(int);



private:
  //Tlist( const Tlist& );
  void insert_all( const Tlist<elemType> &rhs );

  void bump_up_size();
  void bump_down_size();

  int                   interator;
  Tlist_item<elemType>  *_at_front;
  Tlist_item<elemType>  *_at_end;

  Tlist_item<elemType>  *_at_interator;
  int                   _size;
};
//=============================================================================
/*
#define foreach(new_var, list)\
  list.set_interator(0);\
        while(new_var = list.foreach_next())
*/
//=============================================================================
// TFifo declaration
//====================================
template <class Type>
class TFifo
{
public:
  TFifo( int sz)
	{
		max_size = sz;
		size = 0;
		front_ind = 0;
		end_ind = 0;
		Buf = new Type[sz];
	}

  virtual ~TFifo()
  {
	  clear();
	  delete[] Buf;
  }

  Type  pop()
   {
     Type tmp = (Type)0;
     if (size > 0)
     {
       tmp = Buf[front_ind];
       if (++front_ind == max_size)
           front_ind = 0;
       size--;
     }
     else
     {
       front_ind = end_ind = 0;
     }
     return tmp;
   }

   void push(const Type &elem)
   {
       Buf[end_ind] = elem;
       if (++end_ind == max_size)
         end_ind = 0;

       if ( size == max_size)
       {
         if (++front_ind == max_size)
           front_ind = 0;
       }
       else
         size++;
   }

   Type  first()
    {
      Type tmp = (Type)0;
      if (size > 0)
      {
        tmp = Buf[front_ind];
      }
      return tmp;
    }

   Type  last()
    {
      Type tmp = (Type)0;
      if (size > 0)
      {
        tmp = Buf[end_ind];
      }
      return tmp;
    }



   bool empty() const
   {
     return (size == 0);
   }

   void clear(void)
   {
     while(size)
     {
       Buf[size-- -1] = 0;
     }
     front_ind = end_ind = 0;
   }

  unsigned int        size;

private:
  unsigned int        max_size;

  Type			     *Buf;
  unsigned int        front_ind;
  unsigned int        end_ind;
};

}/*namespace*/

#endif /* TYPEDEFINE_H_ */
