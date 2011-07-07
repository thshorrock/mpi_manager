
#ifdef USING_MPI

#include "mpi.hpp"
#include "maths.hpp"



#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
                                  //  a namespace alias is preferred practice in real code
namespace icr = institute_of_cancer_research;
namespace mpi = boost::mpi;
using namespace boost::unit_test;
using namespace icr;
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



void mpi_manager_test()
{
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
     mpi_manager<command_test> mpi( inbox); //this is all that is required
     if (world.rank() ==0) {  
       outbox = mpi.get_outbox();  
       
       for (size_t i = 0; i<size; ++i){
	std::cout<<"result["<<i<<"]  = "<<outbox.front().result()<<std::endl;
        outbox.pop_front();
      }
     }
     
    
     
}


//____________________________________________________________________________//

test_suite*
init_unit_test_suite( int argc, char* argv[] ) 
{
  
  framework::master_test_suite().
    add( BOOST_TEST_CASE( &mpi_manager_test ) );
  //    framework::master_test_suite().
  //        add( BOOST_TEST_CASE( boost::bind(
  //        &test_class::test_method2, tester )));


   return 0;

  
}




#endif
