#ifndef LBL_FSM_H
#define LBL_FSM_H

#include <map>
#include <vector>
#include <assert.h>

namespace lbl
{
    // Forward refence
    //
    template <class T, class EV, class CMP> class Transition;
    template <class T, class EV, class CMP> class State;
    
    //-----------------------------------------------------------------------------
    //
    //  EVENTS
    //
    //      The Event, EV, drives the layout of the State Machine. Events can be
    //      integral types or instances that encapsulate additional information. 
    //      cases will be integral types, but for action handlers that can support
    //      arguments, the Event object may encapsulate additional data. In many
    //      cases this is mandatory. In other cases, the Event object itself may 
    //
    //      The primary use of the Event is to trigger the transaction on a given
    //      state. How events are handled in the system is performed by the event-
    //      driver, which is a front end to the state machine object. It may be
    //      that events are deleted once they are no longer required. The purpose
    //      of this state machine is to drive an object and it's states not mani-
    //      pulate the event object. 
    //
    //      There is no simple solution in making an Event generic. One approach
    //      is to create a WRAPPER CLASS
    //
    //          class MyEvent {
    //              const int hEvent;
    //              SEMAPHORE hLock;
    //
    //              ... // Other specific information to Event Wrappers
    //
    //          public:
    //              MyEvent( const int ev ) : hEvent(ev) {...}
    //
    //              ... // Other methods not listed
    //
    //          public:
    //              char* inbuffer;
    //              mutable char* outbuffer;
    //
    //           };
    //

    //-----------------------------------------------------------------------------
    //
    // ACTIONS
    //
    //      This object represents the action to perform on a transition or on
    //      entry/exit to a new state. 
    //
    //

    template <class T, class EV>
    struct Action
    {
        virtual void operator()( T&, const EV& ) throw() = 0;
    };

    //  Default implemenation of a NULL Action Handler
    template <class T, class EV>
    struct NoAction : public Action<T,EV>
    {
        void operator()(  T&, const EV& ) throw() {};
    };

    // Event Action Handler for a class "T"
    template <class T, class EV>
    struct EventAction : public Action<T,EV>
    {
        typedef void (T::*ACTION)( const EV& ev );

        explicit EventAction( ACTION hAction ) : m_hAction(hAction) {}
        void operator()(T& _T, const EV& ev ) throw() { 
            (_T.*m_hAction)(ev);
        }

    private:
        ACTION m_hAction;
    };

    //-----------------------------------------------------------------------------
    //
    // GUARDS 
    //
    //      This object represents the gard for a trasition to be execute or not.
    //
    //
    template <class T, class EV>
    struct Guard
    {
        virtual bool operator() (const T&, const EV&) = 0;
    };

    template <class T, class EV>
    struct NoGuard : public Guard<T, EV>
    {
        bool operator()(const T&, const EV&) { return true; }
    };

    template <class T, class EV>
    struct EventGuard : public Guard<T, EV>
    {
        typedef bool (T::*GUARD) (const EV& ev) const;
        explicit EventGuard(GUARD hGuard) : m_hGuard(hGuard) {}
        bool operator() (const T& _T, const EV& ev) {
            return (_T.*m_hGuard)(ev);
        }
    private:
        GUARD m_hGuard;
    };

    //-----------------------------------------------------------------------------
    //
    // TRANSITIONS
    //
    //      This object represents the transtions between two states based upon
    //      a generating event in the current state. Transitions are associated
    //      with events only, they do not handle them. They are actually used to
    //      denote the Action associated with the Event (in the current state) and
    //      the New State that we transition to.
    //
    //      Transitions are accessed via a KEY in the State class. This KEY is the
    //      EVENT (typically an integral value, although we don't model events as 
    //      such). They are also only allowed scope within a the State Class, 
    //      therefore only the State Class can create and delete them.
    //

    template <class T, class EV, class CMP>
    class Transition
    {
        friend State<T,EV,CMP>;

    public:
        State<T,EV,CMP>* operator()( const T& _T, const EV& ev ) throw() { 
            if(m_guard == 0 || m_guard != 0 && (*m_guard)(_T, ev)) {
                (*m_evAction)(_T,ev);
                return m_stNext;
            }
            return 0;
        }
        bool isTransitable(const T& _T, const EV& ev) const
        {
            if(m_guard == 0 || m_guard != 0 && (*m_guard)(_T, ev)) 
            {
                return true;
            }
            return false;
        }

    protected:
        Transition( const State<T,EV,CMP>& stNext, 
                Action<T,EV>* evAction, 
                Guard<T,EV>* guard = 0 ) : 
            m_stNext(const_cast<State<T,EV,CMP>*>(&stNext)), 
            m_evAction(evAction), 
            m_guard(guard) 
        {
        }
        ~Transition() { delete m_evAction; }

        // The Next State and Action pair

    private:
        State<T,EV,CMP>* m_stNext;
        Action<T,EV>* m_evAction;
        Guard<T, EV>* m_guard;
    };

    //-----------------------------------------------------------------------------
    //
    //  STATES
    //
    //      CState uses a Sorted Associated Container, map, to store the it's 
    //      State Transitions. The Event, EV, is used as the KEY 
    //
    //      NOTE: Is std::map truly fast enough? We may need to look at a closed
    //      Hashing Scheme based upon the number of transitions per state.
    //
    //      "Hashed Associative Containers" vs "Sort Associative Containers"
    //

    template <class T, class EV, class CMP = std::less<EV> >
    class State
    {
        // State Transition Table Type
        typedef typename EV::EventID event_id;
        typedef std::vector<const Transition<T,EV,CMP>*> TransitionVector;
        typedef std::map<const event_id,
                TransitionVector,CMP> TransitionTable;
    public:
        typedef State<T,EV,CMP> state_type;
        typedef typename EventAction<T,EV>::ACTION ACTION;
        typedef typename EventGuard<T,EV>::GUARD GUARD;

    public:
        State( ACTION hEnter = 0, ACTION hExit = 0 ) {
            if ( hEnter == 0 ) {
                m_evEnter = new NoAction<T,EV>;
            }
            else { 
                m_evEnter = new EventAction<T,EV>(hEnter); 
            }

            if ( hExit == 0 ) {
                m_evExit = new NoAction<T,EV>;
            }
            else {
                m_evExit = new EventAction<T,EV>(hExit); 
            }
        }
        ~State() {
            typename TransitionTable::iterator iter;
            for ( iter = m_stTable.begin(); iter != m_stTable.end(); iter++ ) {
                TransitionVector& tv = iter->second;
                for(typename TransitionVector::iterator iterv = tv.begin(); iterv != tv.end(); ++iterv) {
                    delete const_cast<Transition<T,EV,CMP>*>( *iterv );
                }
            }

            delete m_evEnter;
            delete m_evExit;
        }

        // The API

    public:

        // Adds an entry to the State Transition Table

        void add( const event_id& ev, const state_type& stNext, ACTION hAction = 0, GUARD hGuard = 0) {
            Action<T,EV>* evAction;
            if ( hAction == 0 ) {
                evAction = new NoAction<T,EV>;
            }
            else {
                evAction = new EventAction<T,EV>(hAction); 
            }

            Guard<T, EV>* guard;
            if(hGuard == 0) {
                guard = new NoGuard<T,EV>();
            } else {
                guard = new EventGuard<T, EV>(hGuard);
            }

            m_stTable[ev].push_back(new Transition<T,EV,CMP>(stNext,evAction,guard));
        }

        // Retrieves the Transition from the State Transition Table
        const TransitionVector& operator[]( const event_id& ev) const {
            return m_stTable[ev];
        }

    public:
        void Exit( const T& _T, const EV& ev ) throw() { (*m_evExit)(_T,ev); }
        void Enter( const T& _T, const EV& ev ) throw() { (*m_evEnter)(_T,ev); }

    private:

        TransitionTable m_stTable;

        Action<T,EV>* m_evEnter;
        Action<T,EV>* m_evExit;
    };

    //-----------------------------------------------------------------------------
    //
    //  THE STATE MACHINE
    //
    //      This is the instance of the State Machine for a class "T". It can 
    //      created either by inheritance OR aggregation.
    //
    //
    //                  +-----------------------------+
    //                  |          class T            |
    //                  +--------------+--------------+
    //                                 |
    //                               +---+
    //                                \ /
    //                                 +
    //                                 |
    //                                 |
    //                  +--------------+--------------+
    //                  |    StateMachine<T,EV,CMP>   |
    //                  +-----------------------------+
    //
    //
    //                                OR
    //
    //                  +-----------------------------+
    //                  |          class T            +---------+   
    //                  +-----------------------------+         |
    //                                                          |
    //                                                          |
    //                                                          |
    //                                                          |
    //                                                          |
    //                  +-----------------------------+         |
    //                  |    StateMachine<T,EV,CMP>   |<>-------+
    //                  +-----------------------------+
    //

    // Extended Version of State Machine

#ifdef DERIVE_STATE_MACHINE 

#define THIS            (*this)
    template <class T, class EV, class CMP = std::less<EV> >
    class StateMachine : private T
    {
    public: 
        explicit StateMachine( State<T,EV,CMP>* const stStart ) : 
                T(), m_stStart(stStart), m_stCurrent(stStart), m_stThis(eStopped) {}

        // Aggregated Version of State Machine

#else

#define THIS            (*m_T)
    template <class T, class EV, class CMP = std::less<EV> >
    class StateMachine
    {
    private:
        T* m_T;

        typedef std::vector<const Transition<T,EV,CMP>*> TransitionVector;

    public: 
        explicit StateMachine( T* _T, State<T,EV,CMP>* const stStart ) : 
            m_T(_T), m_stStart(stStart), m_stCurrent(stStart), m_stThis(eStopped) {}
        ~StateMachine() { }

#endif      // DERIVE_STATE_MACHINE

        typedef State<T,EV,CMP> state_type;
        typedef Action<T,EV> action_type;

        // The API

    public:
        enum { eStopped = 0, eRunning };                // Internal States

        int Start( void ) throw() { 
            return (m_stThis != eStopped) ? 
                false : (m_stThis = eRunning); 
        }
        int Halt( void ) throw() { 
            return (m_stThis != eRunning) ? 
                false : (m_stThis = eStopped); 
        }
        int Reset( void ) throw() {
            return (m_stThis != eStopped) ? 
                false : (m_stCurrent = m_stStart); 
        }

        int PostEvent( const EV& event ) throw() { 
            return (m_stThis != eRunning) ? 
                false : ProcessEvent(event); 
        }

    private:
        int ProcessEvent( const EV& event );
        bool transitable(const TransitionVector& tv, const EV& event);

    private:
        int m_stThis;

        State<T,EV,CMP>* const m_stStart;
        State<T,EV,CMP>* m_stCurrent;
    };

    // Returns true if the event was handle (does not imply that the event was
    // valid for current state, but only that the state machine was running and
    // accepting messages).
    //

    template <class T, class EV, class CMP>
    bool StateMachine<T,EV,CMP>::transitable(const TransitionVector& tv, const EV& event)
    {
        for(typename TransitionVector::iter it = tv.begin(); it != tv.end(); ++it)
        {
            if(it->isTransitable(THIS, event))
            {
                return true;
            }
        }
        return false;
    }

    template <class T, class EV, class CMP>
    int StateMachine<T,EV,CMP>::ProcessEvent( const EV& event )
    {
        TransitionVector& trEntry = (*m_stCurrent)[event.getEventID()];

        // Valid Transition?
        if ( trEntry.empty() || !transitable(trEntry, event))
        {
            return false;
        }

        // Invoke State Exit Criteria
        m_stCurrent->Exit(THIS,event);

        for(typename TransitionVector::iter it = trEntry.begin(); it != trEntry.end(); ++it)
        {
            // Invoke Transition (returns new State)
            m_stCurrent = (**it)(THIS,event);
            if(m_stCurrent) {
                break;
            }
        }

        if(m_stCurrent)
        {
            // Invoke State Entry Criteria for new State
            m_stCurrent->Enter(THIS,event);
        } 
        else 
        {
            // Valid Transition?
            assert(false);
            return false;
        }

        return true;
    }
}

#endif
