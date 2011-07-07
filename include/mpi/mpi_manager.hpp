#pragma once


#ifdef USING_MPI

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <sstream>

#include <boost/mpi.hpp>
#include <iostream>
#include <boost/serialization/string.hpp>
#include <boost/serialization/base_object.hpp>
#include <queue>


namespace mpi = boost::mpi;
using std::deque;

//! The main Institute of Cancer Research namespace
namespace institute_of_cancer_research{
  
  //! Constants that label the messages between the processors
  //negatives seem to upset things
  const int MPI_REQUEST_PREDATA = 1;
  const int MPI_RECIEVE_PREDATA = 2;
  const int MPI_REQUEST_JOB = 3;
  const int MPI_RECIEVE_JOB = 4;
  const int MPI_JOB = 5;
  const int MPI_JOBS_END = 6;
  const int MPI_COUNT_OFFSET = 10;
  

  class mpi_command_base{

    size_t m_id;
    bool m_empty; //if sends no data;
    bool m_replied;
    //make it serialisable
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
    {    ar & m_id;    ar & m_empty;    ar & m_replied;  }

  public:
    mpi_command_base(size_t job_id = 0);
    
    virtual 
    ~mpi_command_base();
    
    void 
    set_id(size_t id);
    
    size_t 
    id() const;
    
    size_t 
    get_id() const;
    
    bool 
    empty() const;
    
    void 
    set_empty(bool is_empty = true);

    virtual void run() = 0;
  };

    
  template <class command >
  class mpi_manager
  {
  private:
    deque< command > m_inbox;
    deque< command > m_outbox;
    bool m_display;
    bool m_verbose;
    void run();

  public:
    mpi_manager(){};
    mpi_manager(mpi::communicator& world, const deque< command >& inbox, bool display = false, bool verbose = false);
    //mpi_manager(const deque< command > inbox);
    deque< command > get_outbox(){return m_outbox;};
  };



}

template<class command>
institute_of_cancer_research::mpi_manager<command>::mpi_manager(mpi::communicator& world, const deque< command >& inbox, bool display, bool verbose  )
  : m_inbox(inbox), m_display(display), m_verbose(verbose)
{
  //mpi::environment env;
  // mpi::communicator world;
    
  //if server
  if (world.rank() == 0)
    {
	
      //set-up reply list and count
      deque<mpi::request> replied ;
      size_t count = 0;  //the number of items in the inbox that has been completed
      size_t max_id = 0;
      //the total number of messages to be sent is the size of the inbox  plusa handshake to each of the nodes (-1 because server isn't a node)
      const size_t no_messages = m_inbox.size() + (world.size() -1) ;	
      boost::progress_display* pd;
      if (m_display && !m_verbose) {pd = new boost::progress_display(no_messages); }

      while (count < no_messages ) { 
        //wait for request   //
        
        if(m_verbose)  std::cout<<"SERVER: waiting for request"<<std::endl;
        mpi::status request = world.probe(mpi::any_source, MPI_REQUEST_JOB);
       
        if(m_verbose) std::cout<<"SERVER: request recieved from "<<request.source()<<std::endl;
	  
        //create a job;
        command job;
        if (m_inbox.size() !=0){  //there is a job to send
          job = m_inbox.front() ;  //copy the job from the inbox
	  job.set_id(count);
          m_inbox.pop_front();     //pop the inbox
          //send job	    //
	  if(m_verbose)  std::cout<<"SERVER: sending job to "<<request.source()<<std::endl;	  
          world.send(request.source(),MPI_RECIEVE_JOB, job);
          //create the outbox job
          m_outbox.push_back( job ); //the original job will be altered
          //listen for reply and recieve completed jobs
          replied.push_back( world.irecv(request.source(),job.id() , m_outbox.back() ) );
          //update max_id
          if (job.id() > max_id) {max_id = job.id();}

        }
        else{  //there is no job to send  //
          if(m_verbose) std::cout<<"SERVER: out of jobs!"<<std::endl;
          //flag the job as empty and send
          job.set_empty();
          job.set_id(++max_id); //make sure id is unique
          world.send(request.source(),MPI_RECIEVE_JOB, job);
          //wait for acknowledgement that job is finished (the handshake)
          replied.push_back( world.irecv(request.source(),job.id() , job) );
        }
        ++count;  
        if (m_display && !m_verbose) ++(*pd);
        //close the original job request, the job is done
        world.recv(request.source(),MPI_REQUEST_JOB);
      };
      if (m_display && !m_verbose) delete pd;
      if(m_verbose)  
	std::cout<<"SERVER: Waiting for everything to finish"<<std::endl;
      //wait for all the jobs to be returned (including the empty ones - the handshake)
      mpi::wait_all(replied.begin(), replied.end());
      if(m_verbose) 
	std::cout<<"SERVER: Everything is done"<<std::endl;
    }
  else //   if client
    {
      deque<mpi::request> replied_client ;
      bool still_data = true;  //there are jobs to be done
      while (still_data) { 
        //request a job from server
        world.send(0,MPI_REQUEST_JOB );
        //create a job and wait until it is recieved // 
        if(m_verbose)  std::cout<<"CLIENT "<< world.rank()<<": waiting for a job"<<std::endl;
        command job;
        world.recv(0, MPI_RECIEVE_JOB , job);
        //check to see if there is any data;
        if (job.empty() ) { // 
          if(m_verbose)  std::cout<<"CLIENT "<< world.rank()<<": job recieved is empty"<<std::endl;
          still_data = false;}
        else {
          //we are good to go.  //  
          if(m_verbose) std::cout<<"CLIENT "<< world.rank()<<": job has data"<<std::endl;
          job.run();
        }
        //return data	  
        if(m_verbose) std::cout<<"CLIENT "<< world.rank()<<": replying"<<std::endl;
        //mpi::request r;
        //job.reply(world, r);
        //world.send(0, job.id(), job);
        replied_client.push_back(world.isend(0, job.id(), job ));
        if(m_verbose) std::cout<<"CLIENT "<< world.rank()<<": replyed"<<std::endl;
      };
      // 
      if(m_verbose) std::cout<<"CLIENT "<< world.rank()<<": no more requests"<<std::endl;
    
      //wait for all the jobs to be returned (including the empty ones - the handshake)
      mpi::wait_all(replied_client.begin(), replied_client.end());
    }
  //if(m_verbose) std::cout<<"ALL: reached end"<<std::endl;

  //make sure everything is done and collected together
  //world.barrier();
  
  if(m_verbose) std::cout<<"ALL: Complete"<<std::endl;
  //   std::cout<<"processor "<<world.rank()<<" says ALL: Complete"<<std::endl;
}

BOOST_IS_MPI_DATATYPE(institute_of_cancer_research::mpi_command_base)
//BOOST_IS_MPI_DATATYPE(mpi_manager)
BOOST_CLASS_TRACKING(institute_of_cancer_research::mpi_command_base,track_never)
//BOOST_CLASS_IMPLEMENTATION(mpi_command_base,object_serializable)
//BOOST_CLASS_TRACKING(mpi_manager,track_never)
//BOOST_CLASS_IMPLEMENTATION(mpi_manager,object_serializable)

/* IMPLEMENTATION */


// inline
// icr::mpi_command_base::~mpi_command_base(){
//   if (!m_replied){
//     //std::cout<<"warning: need to reply"<<std::endl;

//     //throw "need to reply";
//     //need to reply, create a world to do so.
//     //This is a bit expensive so its best to use the reply option with a preconstructed world if one exists.
//     //mpi::communicator world;
    
//     //  reply(world);
//   }
// }


// inline  void
// icr::mpi_command_base::reply(mpi::communicator& world, mpi::request& r) {
//   if (m_replied ==false){
//     r = world.isend(0, id(), *this );
//     m_replied = true;
//     }
// }
    

 #else //not using the mpi



#endif

