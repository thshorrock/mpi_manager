
/***********************************************************************************
 ***********************************************************************************
 **                                                                               **
 **  Copyright (C) 2011 Tom Shorrock <t.h.shorrock@gmail.com> 
 **                                                                               **
 **                                                                               **
 **  This program is free software; you can redistribute it and/or                **
 **  modify it under the terms of the GNU General Public License                  **
 **  as published by the Free Software Foundation; either version 2               **
 **  of the License, or (at your option) any later version.                       **
 **                                                                               **
 **  This program is distributed in the hope that it will be useful,              **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of               **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                **
 **  GNU General Public License for more details.                                 **
 **                                                                               **
 **  You should have received a copy of the GNU General Public License            **
 **  along with this program; if not, write to the Free Software                  **
 **  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  **
 **                                                                               **
 ***********************************************************************************
 ***********************************************************************************/


#define BOOST_TEST_MODULE mpi_manager_library_tests


#include "mpi_manager.hpp"
//#include "maths.hpp"



#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "dimensioned_math/rng.hpp"
#include "dimensioned_math/vec.hpp"

using namespace boost::unit_test;

namespace mpi = boost::mpi;
using namespace ICR;
//____________________________________________________________________________//

  
struct B
{
  virtual void set(double d) = 0;
  virtual double get() const = 0;
private:
    friend class boost::serialization::access;
    // no real serialization required - specify a vestigial one
    template<class Archive>
    void serialize(Archive & ar, const unsigned int file_version){}
};

struct T : public B
{
  T(double d = 2.0) : B(), m_d(d) {}
  double m_d;
  ICR::vec m_v;
  maths::rng m_r;
  
  friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
  {    
        boost::serialization::base_object<B>(*this);
    ar& m_d;
  }
    
  void set(double d) {m_d = d;}
  double get() const {return m_d;}
  
};

BOOST_CLASS_EXPORT_GUID(T, "T");

class test_job{ 
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
  {    ar & m_result;  
    ar& m_t;
  }
  //mpi::communicator world;
  int m_result;
  boost::shared_ptr<B> m_t;

public:

  void attach(double d)
  {
    boost::shared_ptr<T> t(new T(d));
    m_t = t;
  }
  void print() {mpi::communicator world; std::cout<<"Hello from process "<<world.rank()<<std::endl;};
  void set_result(){mpi::communicator world; m_result = world.rank() ;
    m_t->set(4);
  };
  double result(){ return m_result ; };
};

//class< T >
class command_test : public  mpi_command_base, public test_job
{  
friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        // serialize base class information
        ar & boost::serialization::base_object<mpi_command_base>(*this);
        ar & boost::serialization::base_object<test_job>(*this);
    }

public:
  command_test(size_t job_id = 0) : 
    mpi_command_base(job_id),
    test_job()
  {};
  void run() {print(); 
    set_result();};
};



BOOST_AUTO_TEST_SUITE( mpi_manager_test )

BOOST_AUTO_TEST_CASE( general_test  )
{
  int a = 1;
  
  // void mpi_manager_test()
  // {
  //std::cout<<"Testing the MPI library:  This should not take a long time, if you have a long pause now, check the connections between the pcs and maybe do an mpdtrace..."<<std::endl;

  mpi::environment env;
  mpi::communicator world;
  
  size_t size = 5;
  deque<command_test> inbox;
  deque<command_test> outbox;
  deque<command_test> outbox2;

  if (world.rank() ==0) {
    for (size_t i = 0; i<size; ++i){
      command_test cmd( i);
      cmd.attach(3);
      //cmd.run()
      inbox.push_back( cmd  );
    }
  }
  mpi_manager<command_test> mpi(world, inbox); //this is all that is required
  if (world.rank() ==0) {  
    outbox = mpi.get_outbox();  
       
    for (size_t i = 0; i<size; ++i){
      std::cout<<"result["<<i<<"]  = "<<outbox.front().result()<<std::endl;
      outbox.pop_front();
    }
  }
     
  // }
}

BOOST_AUTO_TEST_SUITE_END()

// //____________________________________________________________________________//

// test_suite*
// init_unit_test_suite( int argc, char* argv[] ) 
// {
  
//   framework::master_test_suite().
//     add( BOOST_TEST_CASE( &mpi_manager_test ) );
//   //    framework::master_test_suite().
//   //        add( BOOST_TEST_CASE( boost::bind(
//   //        &test_class::test_method2, tester )));


//    return 0;

  
// }




