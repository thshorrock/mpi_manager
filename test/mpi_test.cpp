
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

using namespace boost::unit_test;

#ifdef USING_MPI

namespace mpi = boost::mpi;
using namespace ICR;
//____________________________________________________________________________//

class test_job{ 
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
  {    ar & m_result;  
  }
  //mpi::communicator world;
  int m_result;

public:
  void print() {mpi::communicator world; std::cout<<"Hello from process "<<world.rank()<<std::endl;};
  void set_result(){mpi::communicator world; m_result = world.rank() ;};
  double result(){ return m_result ;};
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
  void run() {print(); set_result();};
};

#endif


BOOST_AUTO_TEST_SUITE( mpi_manager_test )

BOOST_AUTO_TEST_CASE( general_test  )
{
  int a = 1;
  
#ifdef USING_MPI
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
     
# endif
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




