#pragma once

#ifdef USING_MPI


#include <sstream>

#include <boost/mpi.hpp>
#include <iostream>
#include <boost/serialization/string.hpp>

#include <queue>


namespace mpi = boost::mpi;
using std::deque;

//! The main Institute of Cancer Research namespace
namespace ICR{

  enum {
    MPI_REQUEST_JOB,
    MPI_RECIEVE_JOB,
    MPI_JOB
  };
  

  class mpi_command_base{
    //make it serialisable
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
    {    ar & m_id;    ar & m_empty;    ar & m_replied;  }

    size_t m_id;
    bool m_empty; //if sends no data;
    bool m_replied; 
    //mpi::environment& m_env;
    //mpi::communicator m_world;
  public:
    mpi_command_base(size_t job_id = 0 , bool empty = true) 
      :  m_id(job_id), m_empty(empty), m_replied(false) {};
    virtual ~mpi_command_base(){ 
      //reply();
    };
    //mpi_command<T>& void set_task(T& task) {m_job = task; m_empty=false; };
    //void set_task (mpi_command_base& other){ *this = other; };
    void set_id(size_t id){m_id = id;};
    size_t get_id(){return m_id;};
    bool empty(){return m_empty;}; 
    virtual void run() = 0;
    void reply(mpi::communicator& world) {
      if (m_replied ==false){
	world.isend(0, MPI_JOB, *this );
	m_replied = true;
      }
    };

  };




  class 
  mpi_request_job{
    //make it serialisable
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
    {    ar & m_id;   }
  
    size_t m_id;
    //      mpi::environment env;
    //mpi::communicator world;
  public:
     mpi_request_job(mpi::communicator& world) : m_id( world.rank() ) { world.isend(0, MPI_REQUEST_JOB, *this ); };
    size_t id() const {return m_id;};
  };


  
  template <class command >
  class mpi_manager
  {
  private:
    deque< command > m_inbox;
    deque< command > m_outbox;
  

  public:
    mpi_manager(){};
    mpi_manager(deque< command >& inbox, mpi::communicator& world   );
    void set_inbox(deque< command >& inbox) {m_inbox = inbox;};
    deque< command > get_outbox(){return m_outbox;};
  };


  
  template<class command>
  mpi_manager<command>::mpi_manager(deque< command >& inbox, mpi::communicator& world   )
    : m_inbox(inbox)
  {
    
    // mpi::communicator world;
    
    //if server
    if (world.rank() == 0)
      {
	//set-up reply list and count
 	deque<mpi::request> replied ;
 	size_t count = 0;
 	//	size_t open_nodes = world.size() - 1; //-1 because server isn't a node
	while (count < m_inbox.size() + m_outbox.size() + world.size() ) {
 	  //total size + a handshake
 	  //wait for request
	  mpi::status request = world.probe(mpi::any_source, MPI_REQUEST_JOB);
 	  //mpi_request_job request(world);
	  //  world.irecv(1, MPI_RECIEVE_JOB , request);
	  //	  world.recv(1,0,request); 
 	  //create a job;
 	  command job(count);
 	  if (m_inbox.size() !=0){
 	    job = m_inbox.front() ;
 	    m_inbox.pop_front();
	  }
	  // 	  else //nothing to process, send an empty job
	  //  job.set_id(count);
	  
 	  //send job	  
 	  world.isend(request.source(), count, job);
	  
 	  //listen for reply
 	  m_outbox.push_back( job ); //the original job will be altered
 	  replied[count] = world.irecv(request.source(), count, m_outbox.back() );
 	  ++count;
 	};
// 	mpi::wait_all(replied.begin(), replied.end());
      }
    else   //if client
      {
	bool still_data = true;
	while (still_data){
	  //request      
	  
	  //mpi_request_job job_please(world);
	  world.send(0, MPI_REQUEST_JOB);
	  //wait for job
	  command job;
	  world.recv(0, MPI_REQUEST_JOB , job);
	  //check to see if there is any data;
	  if (job.empty() ) {still_data = false;}
	  else {
	    //we are good to go.
	    job.run();
	  }
	  //return data
	  job.reply(world);
	};
    
      }


  }

}


#endif



//template <class T> 
//class mpi_job{
//
//};


// template <class T>
// class mpi_batch{
//   deque<T> m_batch;
//   size_t m_id; //the id of where the job is going
// public:
//   batch(size_t id = 0) : m_batch(), m_id(id) {};
//   void push_job(T& job){m_batch.push_back(job);};
//   T pop_job(void) { T ret = m_batch.front(); m_batch.pop_front(); return ret; };
// };


// class
// mpi_request_batch{
//   const size_t m_id;
//   const size_t m_size;
// public:
//   mpi_request_batch(size_t size) : m_id(world.id()), m_size(size) {
//     world.isend(0, MPI_REQUEST_BATCH,*this );
// };
//   size_t id() const {return m_id;};
//   size_t size() const {return m_size;};

// };


// class mpi_close_connection{
//   const size_t m_id;
// public:
//   mpi_close_connection() : m_id(world.id()) {};
//   size_t id() const {return m_id;};
// };
  


  



// class mpi_request_jobs{
//   const size_t m_id;
//   size_t m_jobs;
// public:
//   mpi_request_jobs(size_t jobs = 1) : m_id(world.rank()), m_jobs(jobs) {};
//   size_t size(){return m_jobs;};
//   size_t id(){return m_id;};
// };


// class mpi_batch{
//   size_t m_id;
//   deque<double> m_batch;
  
// public:
//   double size(){return m_batch.size();};
//   double get_front(){double d = m_batch.front(); m_batch.pop_front(); return d;};
//   size_t push_back(){m_batch.push_back();return m_id;};
//   void push_back(deque<double>& messages_loc, deque<double>& messages, const size_t number)
//   { for (size_t i = 0; i<number;++i) {
//       if (main.size() ==0) break
//       m_batch.push_back(main.front());
//       messages.pop_front();
//       messages_loc.push_back(m_id);
//     };
//   };
//   mpi_batch(size_t id): m_id( id);
//   mpi_batch(size_t id, deque<double>& messages_loc, deque<double>& messages, const size_t number)
//     : m_id(id)
//   {
//     push_back(messages_loc,messages , number);
//   } 
// };


// enum {
//   MPI_REQUEST_N_JOBS,
//   MPI_RECIEVE_JOBS
// }

// class mpi_manager{

//   deque<double> messages;
//   deque<size_t> message_loc;
//   size_t batch_count;
//   deque<mpi::request> batch_status
// public:
//   void add_message(double m){
//     if (is_node) return;
//     messages.push_back(m);
//   };
//   size_t get_id() {return world.rank();};
//   bool   is_node(){return if(get_id()==0) return false; else return true;}; 
//   void request(size_t no_jobs){
//     if (is_node()) {
    
//       size_t id = get_id();
//       request_jobs = mpi_request_jobs(no_jobs);
//       world.isend(0, MPI_REQUEST_N_JOBS,request_jobs);
//       mpi_batch batch;
//       world.recv(0, MPI_RECIEVE_JOBS , batch);
//       return batch;
//     }
//     else {
//       //get the number requested;
//       mpi_request_jobs job_request;
//       world.recv(0, MPI_REQUEST_N_JOBS,job_request);
//       //create a batch
//       mpi_batch batch(job_request.id(),message_loc, messages, job_request.size());
//       ++batch_count;
//       //send the job
//       mpi::request r = world.isend(job_request.id(), MPI_RECIEVE_JOBS, batch);
//       batch_status.push_back(r);
//       return batch;
//     }
//   }
//   mpi_manager():messages() batch_count(0),batch_status)();
//   ~mpi_manager(){
//     mpi::wait_all(batch_status.begin(),batch_status.end());
//   }

// }
