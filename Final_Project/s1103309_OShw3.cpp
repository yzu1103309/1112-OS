#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <queue>
using namespace std;

/* Values that can be changed */
int real_ms = 1500; // control the speed of the program
#define SEED 0  // the seed for srand()

/* The labels for neat output */
#define ERROR_FLAG "[\e[1;31m  ERROR  \e[1;37m] "
#define PRIOR_FLAG "[\e[1;31m  PRIOR  \e[1;37m] "
#define WASTE_FLAG "[\e[1;31m DISCARD \e[1;37m] "
#define FREEZER_FLAG "[\e[1;37m Freezer \e[1;37m] "
#define WAITING_FLAG "[\e[1;33m Waiting \e[1;37m] "
#define CUT_Ent_FLAG "[\e[1;34m Cut_Ent \e[1;37m] "
#define CAN_Ent_FLAG "[\e[1;35m Can_Ent \e[1;37m] "
#define CUT_Out_FLAG "[\e[1;36m Cut_Out \e[1;37m] "
#define CAN_Out_FLAG "[\e[1;32m Can_Out \e[1;37m] "
#define CLOSE_FLAG "[\e[1;32m  CLOSE  \e[1;37m] "
#define STATUS_Flag "[\e[1;37m  -----  \e[1;37m] "

class fish
{
public:
    fish(int id_in)
    {
        id = id_in;
    }
    pthread_t fr_thread;    // for freezer
    int freeze_start;
    int freeze_end;
    int id;
    bool prior = false;
};

// mutex locks for I/O, sots, q_cut, q_can, q_prior (respectively)
pthread_mutex_t io_lock, sl_lock, q_cut_lock, q_can_lock, q_prior_lock;

// fish count = argv[1]
// sl_count default: 5 slots, locked by sl_lock
int fish_count, slot_count = 5, to_cut = 0, to_can = 0, discarded = 0;

// the current time, only modified by Timer
// next_check: next timing to check factory status
int time_ms = 0, next_check = 0;

// for producing new fish
int current_fishID = 1, next_produce;

// 2 queues (for cutting and canning), but only N slots in total
// q_prior: the queue for high priority
queue<fish> q_cut, q_can, q_prior;

bool cut_working = false, can_working = false;

// temp slot for cut fish, swap with the next fish entered
fish cut_tmp = fish(0);

// function for cut factory
void * cut( void * ptr);

// function for can factory
void * can( void * ptr );

// function for Timer
void * time( void * ptr );

// function for each freezer space
void * freeze( void * ptr );

// initial the mutex locks
void init_locks();

class cut_factory
{
public:
    cut_factory()
    {
        // create a new thread once the factory is built
        pthread_create(&threadID, nullptr, cut, nullptr);
    }
    pthread_t threadID;
};

class can_factory
{
public:
    can_factory()
    {
        // create a new thread once the factory is built
        pthread_create(&threadID, nullptr, can, nullptr);
    }
    pthread_t threadID;
};

class Timer // calc time and meanwhile produce new fish
{
public:
    Timer()
    {
        // create a new thread once the Timer is created
        pthread_create(&threadID, nullptr, time, nullptr);

        // set random seed
        srand(SEED);

        //decide when the first fish enters
        next_produce = rand()%6 + 5;
    }
    pthread_t threadID;
};

Timer timer;

int main(int argc, char * argv[])
{
    init_locks();
    if(argc <= 1)
    {
        pthread_mutex_lock(&io_lock);
        cout << ERROR_FLAG << "one or more arguments are needed" << endl;
        pthread_mutex_unlock(&io_lock);
        exit(EXIT_FAILURE);
    }
    else{
        fish_count = atoi(argv[1]);
        to_cut = fish_count;
        to_can = fish_count;
        if(argc >= 3)
            slot_count = atoi(argv[2]);
    }

    cut_factory A;
    can_factory B;

    // wait for the factory to end their jobs
    pthread_join(A.threadID, nullptr);
    pthread_join(B.threadID, nullptr);
}

void * cut( void * ptr)
{
    while(true)
    {
        if( !q_cut.empty() )
        {
            cut_working = true;

            // fish enters the factory
            pthread_mutex_lock(&q_cut_lock);
            fish f = q_cut.front();
            q_cut.pop();
            pthread_mutex_unlock(&q_cut_lock);

            int cut_time = rand()%21 + 10;
            int end_time = time_ms + cut_time;

            pthread_mutex_lock(&io_lock);
            cout << CUT_Ent_FLAG << time_ms << "ms: Fish " << f.id << " enters the cut factory, need " << cut_time << "ms" << endl;
            pthread_mutex_unlock(&io_lock);

            pthread_mutex_lock(&sl_lock);
            pthread_mutex_lock(&q_prior_lock);
            ++slot_count;   // release a slot
            // if there's a cut fish waiting on the machine
            if(cut_tmp.id != 0)
            {
                --slot_count;   // take the slot
                // take the fish from the machine to slot
                pthread_mutex_lock(&q_can_lock);
                q_can.push( cut_tmp );
                pthread_mutex_unlock(&q_can_lock);

                pthread_mutex_lock(&io_lock);
                cout << WAITING_FLAG << time_ms << "ms: Fish " << cut_tmp.id << " back to slot (already cut), waiting to be canned" << endl;
                pthread_mutex_unlock(&io_lock);

                cut_tmp = fish( 0 );
            }
            // if no fish waiting on machine, but there are high priority ones
            else if(!q_prior.empty())
            {
                --slot_count;   // take the slot
                fish f_p = q_prior.front();
                q_prior.pop();

                pthread_mutex_lock(&q_cut_lock);
                q_cut.push(f_p);
                pthread_mutex_unlock(&q_cut_lock);

                pthread_mutex_lock(&io_lock);
                cout << WAITING_FLAG << time_ms << "ms: Fish " << f_p.id << " enters the slot (high priority), waiting to be cut" << endl;
                pthread_mutex_unlock(&io_lock);
            }
            pthread_mutex_unlock(&q_prior_lock);
            pthread_mutex_unlock(&sl_lock);

            while(time_ms < end_time); // wait until the end time

            pthread_mutex_lock(&io_lock);
            cout << CUT_Out_FLAG << time_ms << "ms: Fish " << f.id << " left the cutter." << endl;
            pthread_mutex_unlock(&io_lock);

            --to_cut;
            // temporarily store the cut fish, handle once there's an empty slot
            cut_tmp = f;
            cut_working = false;
        }
        // check if the thread should be ended
        if(to_cut - discarded == 0)
        {
            pthread_mutex_lock(&io_lock);
            cout << CLOSE_FLAG << time_ms << "ms: Nothing more to be cut, shutting down the cutter..." << endl;
            pthread_mutex_unlock(&io_lock);
            break;
        }
    }
    return nullptr;
}

// almost the same with the cut factory
void * can( void * ptr )
{
    while(true)
    {
        if( !q_can.empty() )
        {
            can_working = true;
            pthread_mutex_lock(&q_can_lock);
            fish f = q_can.front();
            q_can.pop();
            pthread_mutex_unlock(&q_can_lock);

            int can_time = rand()%51 + 50;
            int end_time = time_ms + can_time;

            pthread_mutex_lock(&io_lock);
            cout << CAN_Ent_FLAG << time_ms << "ms: Fish " << f.id << " enters the canning factory, need " << can_time << "ms" << endl;
            pthread_mutex_unlock(&io_lock);

            pthread_mutex_lock(&sl_lock);
            pthread_mutex_lock(&q_prior_lock);
            ++slot_count;
            if(cut_tmp.id != 0)
            {
                --slot_count;

                pthread_mutex_lock(&q_can_lock);
                q_can.push( cut_tmp );
                pthread_mutex_unlock(&q_can_lock);

                pthread_mutex_lock(&io_lock);
                cout << WAITING_FLAG << time_ms << "ms: Fish " << cut_tmp.id << " back to slot (cut), waiting to be canned" << endl;
                pthread_mutex_unlock(&io_lock);

                cut_tmp = fish( 0 );
            }
            else if(!q_prior.empty())
            {
                --slot_count;
                fish f_p = q_prior.front();
                q_prior.pop();

                pthread_mutex_lock(&q_cut_lock);
                q_cut.push(f_p);
                pthread_mutex_unlock(&q_cut_lock);

                pthread_mutex_lock(&io_lock);
                cout << WAITING_FLAG << time_ms << "ms: Fish " << f_p.id << " enters the slot (high priority), waiting to be cut" << endl;
                pthread_mutex_unlock(&io_lock);
            }
            pthread_mutex_unlock(&q_prior_lock);
            pthread_mutex_unlock(&sl_lock);

            while(time_ms < end_time); // wait until the end time

            pthread_mutex_lock(&io_lock);
            cout << CAN_Out_FLAG << time_ms << "ms: Fish " << f.id << " left the canner (Complete)." << endl;
            pthread_mutex_unlock(&io_lock);

            --to_can;

            can_working = false;
        }
        // determine whether the program should be ended
        if(to_can - discarded == 0)
        {
            pthread_mutex_lock(&io_lock);
            cout << CLOSE_FLAG << time_ms << "ms: All fish are handled properly, end of the job." << endl;
            pthread_mutex_unlock(&io_lock);

            exit( 0 );
        }
    }
}

void * time( void * ptr )
{
    while(true)
    {
        if(time_ms == next_check) // check if factories idle periodically
        {
            next_check += 10; // period
            if(!cut_working && !can_working)
            {
                pthread_mutex_lock(&io_lock);
                cout <<STATUS_Flag << time_ms << "ms: Cutter and Canner are under reviewing together" << endl;
                pthread_mutex_unlock(&io_lock);
            }
            else if(!cut_working && to_cut - discarded > 0)
            {
                pthread_mutex_lock(&io_lock);
                cout << STATUS_Flag << time_ms << "ms: Cutter is under maintenance" << endl;
                pthread_mutex_unlock(&io_lock);
            }
            else if(!can_working && to_can - discarded > 0)
            {
                pthread_mutex_lock(&io_lock);
                cout << STATUS_Flag << time_ms << "ms: Canner is under maintenance" << endl;
                pthread_mutex_unlock(&io_lock);
            }
        }

        // error checker
        if( slot_count < 0)
        {
            cout << ERROR_FLAG << "An unexpected error occurred (slot_count fault)" << endl;
            exit(EXIT_FAILURE);
        }

        // increment the time stamp after specific period of time
        usleep(real_ms);
        ++time_ms;

        // check if some high priority fish exceed time limit
        pthread_mutex_lock(&q_prior_lock);
        if(!q_prior.empty())
        {
            fish f = q_prior.front();
            if(time_ms >= f.freeze_end)
            {
                q_prior.pop();

                ++discarded;

                pthread_mutex_lock(&io_lock);
                cout << WASTE_FLAG << time_ms << "ms: Fish " << f.id << " Waited for too long. Discarded" << endl;
                pthread_mutex_unlock(&io_lock);
            }
        }
        pthread_mutex_unlock(&q_prior_lock);

        // produce new fish
        if(current_fishID <= fish_count)
        {
            if(time_ms == next_produce)
            {
                // decide when the next fish should enter
                next_produce += rand()%6 + 5;

                pthread_mutex_lock(&sl_lock);
                if(slot_count > 0)  // there are empty slots
                {
                    pthread_mutex_lock(&q_cut_lock);
                    q_cut.push( fish(current_fishID) );
                    pthread_mutex_unlock(&q_cut_lock);

                    pthread_mutex_lock(&io_lock);
                    cout << WAITING_FLAG << time_ms << "ms: Fish " << current_fishID << " (NEW) waiting in slot to enter cut factory" << endl;
                    pthread_mutex_unlock(&io_lock);

                    --slot_count;
                }
                else  // no empty slot, send in freezer
                {
                    fish tmp(current_fishID);
                    int freeze_time = rand()%21 + 30;   // the time to stay in freezer
                    tmp.freeze_end = time_ms + freeze_time; // the time when the fish should leave the freezer

                    // each freezer space is a independent thread, making the process easier
                    pthread_create( &tmp.fr_thread, nullptr, freeze, reinterpret_cast<void *>(&tmp) );

                    pthread_mutex_lock(&io_lock);
                    cout << FREEZER_FLAG << time_ms << "ms: No empty slot, Fish " << current_fishID << " (NEW) Enters the freezer for "  << freeze_time << "ms" << endl;
                    pthread_mutex_unlock(&io_lock);
                }
                pthread_mutex_unlock(&sl_lock);
                ++current_fishID;
            }
        }
    }
}

void * freeze( void * ptr )
{
    fish f = *(reinterpret_cast<fish *>(ptr));
    f.freeze_start = time_ms;
    while(true)
    {
        // check periodically whether slots available
        if(time_ms >= f.freeze_end)
        {
            pthread_mutex_lock(&sl_lock);
            if(slot_count > 0)  // slots available
            {
                pthread_mutex_lock(&io_lock);
                cout << WAITING_FLAG << time_ms << "ms: Fish " << f.id << " left the freezer, waiting in slot to enter cutting factory" << endl;
                pthread_mutex_unlock(&io_lock);

                pthread_mutex_lock(&q_cut_lock);
                q_cut.push( f );
                pthread_mutex_unlock(&q_cut_lock);
                --slot_count;
            }
            else
            {
                // if the fish has stayed in the freezer for too long
                if(f.freeze_end - f.freeze_start >= 600)
                {
                    f.freeze_end = time_ms + 300;
                    f.prior = true;
                    pthread_mutex_lock(&q_prior_lock);
                    q_prior.push( f );
                    pthread_mutex_unlock(&q_prior_lock);

                    pthread_mutex_lock(&io_lock);
                    cout << PRIOR_FLAG << time_ms << "ms: Fish " << f.id << " left the freezer (frozen for too long). Set higher priority" << endl;
                    pthread_mutex_unlock(&io_lock);
                }
                else    // can stay in the freezer for longer
                {
                    int freeze_time = rand()%21 + 30;
                    f.freeze_end = time_ms + freeze_time;

                    pthread_mutex_lock(&io_lock);
                    cout << FREEZER_FLAG << time_ms << "ms: Still no empty slot. Fish " << f.id << " stay in freezer for another "  << freeze_time << "s" << endl;
                    pthread_mutex_unlock(&io_lock);
                }
            }
            pthread_mutex_unlock(&sl_lock);

            // break if the fish left the freezer
            if(time_ms >= f.freeze_end || f.prior) break;
        }
    }
    return nullptr;
}

void init_locks()
{
    pthread_mutex_init(&io_lock, nullptr);
    pthread_mutex_init(&sl_lock, nullptr);
    pthread_mutex_init(&q_cut_lock, nullptr);
    pthread_mutex_init(&q_can_lock, nullptr);
    pthread_mutex_init(&q_prior_lock, nullptr);
}
