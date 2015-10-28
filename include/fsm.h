#ifndef LBL_FSM_H
#define LBL_FSM_H

#include <map>

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
        virtual void operator()( const T&, const EV& ) throw() = 0;
    };

    //  Default implemenation of a NULL Action Handler
    template <class T, class EV>
    struct NoAction : public Action<T,EV>
    {
        void operator()(  const T&, const EV& ) throw() {};
    };

    // Event Action Handler for a class "T"
    template <class T, class EV>
    struct EventAction : public Action<T,EV>
    {
        typedef void (T::*HANDLE)( const EV& ev );

        explicit EventAction( HANDLE hAction ) : m_hAction(hAction) {}
        void operator()( const T& _T, const EV& ev ) throw() { 
            (_T.*m_hAction)(ev);
        }

    private:
        HANDLE m_hAction;
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
        typedef void (T::*HANDLE) (const EV& ev);
        explicit EventGuard(HANDLE hGuard) : m_hGuard(hGuard) {}
        bool operator() (const T& _T, const EV& ev) {
            return (_T.*m_hGuard)(ev);
        }
    private:
        HANDLE m_hGuard;
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
            (*m_evAction)(_T,ev);
            return m_stNext;
        }

    protected:
        Transition( const State<T,EV,CMP>& stNext, Action<T,EV>* evAction ) : 
            m_stNext(const_cast<State<T,EV,CMP>*>(&stNext)), m_evAction(evAction) {
        }
        ~Transition() { delete m_evAction; }

        // The Next State and Action pair

    private:
        State<T,EV,CMP>* m_stNext;
        Action<T,EV>* m_evAction;
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

        typedef std::map<const EV,const Transition<T,EV,CMP>*,CMP>
            TransitionTable;

    public:
        typedef State<T,EV,CMP> state_type;
        //typedef EventAction<T,EV>::HANDLE HANDLE;
        typedef void (T::*HANDLE)( const EV& ev );

    public:
        State( HANDLE hEnter = 0, HANDLE hExit = 0 ) {
            if ( hEnter == 0 ) {
                m_evEnter = new NoAction<T,EV>;
            }
            else { 
                m_evEnter = new EventAction<T,EV>(hEnter); 
            }

            if ( hExit == 0 ) {
                m_evExit = new NoAction<T,EV>;
            }
            else { m_evExit = new EventAction<T,EV>(hExit); }
        }
        ~State() {
            TransitionTable::iterator iter;
            for ( iter = m_stTable.begin(); iter != m_stTable.end(); iter++ ) {
                delete const_cast<Transition<T,EV,CMP>*>( iter->second );
            }
            delete m_evEnter;
            delete m_evExit;
        }

        // The API

    public:

        // Adds an entry to the State Transition Table

        void Add( const EV& ev, const state_type& stNext, HANDLE hAction = 0 ) {
            Action<T,EV>* evAction;
            if ( hAction == 0 ) {
                evAction = new NoAction<T,EV>;
            }
            else {
                evAction = new EventAction<T,EV>(hAction); 
            }
            m_stTable[ev] = new Transition<T,EV,CMP>(stNext,evAction);
        }

        // Retrieves the Transition from the State Transition Table

        Transition<T,EV,CMP>* const operator[]( const EV& ev) {
            return const_cast<Transition<T,EV,CMP>*>(m_stTable[ev]);
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

    public: 
        explicit StateMachine( T* _T, State<T,EV,CMP>* const stStart ) : 
            m_T(_T), m_stStart(stStart), m_stCurrent(stStart), m_stThis(eStopped) {}
        ~StateMachine() { delete m_T; }

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
    int StateMachine<T,EV,CMP>::ProcessEvent( const EV& event )
    {
        Transition<T,EV,CMP>* const trEntry = (*m_stCurrent)[event];

        // Valid Transition?

        if ( !trEntry )
        {
            return false;
        }

        // Invoke State Exit Criteria

        m_stCurrent->Exit(THIS,event);

        // Invoke Transition (returns new State)

        m_stCurrent = (*trEntry)(THIS,event);

        // Invoke State Entry Criteria for new State

        m_stCurrent->Enter(THIS,event);

        return true;
    }

}

#endif
