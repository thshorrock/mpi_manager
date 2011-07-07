
//#ifdef USING_MPI

#include "mpi.hpp"



#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
//#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
//#include <iostream>               // for std::cout
//namespace fs =  boost::filesystem;          // for ease of tutorial presentation;
                                  //  a namespace alias is preferred practice in real code
namespace icr = institute_of_cancer_research;
namespace mpi = boost::mpi;
using namespace boost::unit_test;
using namespace icr;
//____________________________________________________________________________//

class test_job{ 
    mpi::communicator world;
public:
  void print() {std::cout<<"Hello from process "<<world.rank()<<std::endl;};
};

//class< T >
class command_test : public  mpi_command_base, public test_job
{  
public:
  command_test(size_t job_id = 0) : 
    mpi_command_base(job_id, false),
    test_job()
  {};
  void run() {print();};
};


// void mpi_test()
// {
//   //test to see whether boost mpi is working at all...
  


// }

void mpi_manager_test()
{
std::cout<<"Testing the MPI library:  This should not take a long time, if you have a long pause now, check the connections between the pcs and maybe do an mpdtrace..."<<std::endl;

  mpi::environment env;
  mpi::communicator world;

  deque<command_test> inbox;
  deque<command_test> outbox;
   for (size_t i = 0; i<100; ++i){
    command_test cmd( i);
    inbox.push_back( cmd  );
   }
   mpi_manager<command_test> mpi( inbox, world); //this is all that is required

//   outbox = mpi.get_outbox();




//   for (size_t i = 0; i<100; ++i){
//     inbox.pop_front();
//   }
     
       

}


//____________________________________________________________________________//

test_suite*
init_unit_test_suite( int argc, char* argv[] ) 
{
  
  //boost::shared_ptr<test_class> tester( new test_class );

  //MPI_Init(&argc, &argv);
  
  framework::master_test_suite().
    add( BOOST_TEST_CASE( &mpi_manager_test ) );
  //    framework::master_test_suite().
  //        add( BOOST_TEST_CASE( boost::bind(
  //        &test_class::test_method2, tester )));

  //MPI_Finalize();

   return 0;

  
}




//#endif
