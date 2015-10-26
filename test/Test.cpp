
class XXXService
{
public:
    class StateMachine : public lbl::StateMachineDef<StateMachine> {
        // Define the state
        struct Idle : public lbl::State {
            template <class Event, class FSM>
            void onEntry(Event const&, FSM&) {
            }
            template <class Event, class FSM>
            void onExit(Event const&, FSM&) {
            }
        };
        struct EstablishingUnstable : public lbl::State {
            template <class Event, class FSM>
            void onEntry(Event const&, FSM&) {
            }
            template <class Event, class FSM>
            void onExit(Event const&, FSM&) {
            }
        };
        struct Established : public lbl::State {
            template <class Event, class FSM>
            void onEntry(Event const&, FSM&) {
            }
            template <class Event, class FSM>
            void onExit(Event const&, FSM&) {
            }
        };
        struct TerminatingUnstable: public lbl::State {
            template <class Event, class FSM>
            void onEntry(Event const&, FSM&) {
            }
            template <class Event, class FSM>
            void onExit(Event const&, FSM&) {
            }
        };
        struct Terminated: public lbl::State {
            template <class Event, class FSM>
            void onEntry(Event const&, FSM&) {
            }
            template <class Event, class FSM>
            void onExit(Event const&, FSM&) {
            }
        };

        typedef Idle IntialState;

        // Define transition actions
        struct FromSignallingOngoingToEstablished
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            void operator() (Event const&, FSM&, SourceState&, TargetState&) {
            }
        };
        struct FromEstablishedToTerminating
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            void operator() (Event const&, FSM&, SourceState&, TargetState&) {
            }
        };
        struct FromTerminatingToFinal
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            void operator() (Event const&, FSM&, SourceState&, TargetState&) {
            }
        };
        struct SignallingOngoing
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            void operator() (Event const&, FSM&, SourceState&, TargetState&) {
            }
        };

        // Define Guard conditions
        struct TargetIsIncomingDialog
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            bool operator() (Event const&, FSM&, SourceState&, TargetState&) {
                return true;
            }
        };
        struct SameCSeqAsIntialInvite
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            bool operator() (Event const&, FSM&, SourceState&, TargetState&) {
                return true;
            }
        };
        struct TargetIsOutgoingDialog
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            bool operator() (Event const&, FSM&, SourceState&, TargetState&) {
                return true;
            }
        };
        struct EventSourceIsNotService 
        {
            template <class Event, class FSM, class SourceState, class TargetState>
            bool operator() (Event const&, FSM&, SourceState&, TargetState&) {
                return true;
            }
        };

        typedef StateMachine S;


        struct TransitionTable : lbl:vector<
            //   Start                         Event                           Next            Action                               Guard
            // +--------------------+------------------------------+---------------------+-----------------------------------+------------------------+
            Row<Idle                , IInviteEvent                 , Idle                , none                              , none                   ,
            Row<Idle                , IProvisionalResponseEvent    , EstablishingUnstable, none                              , TargetIsIncomingDialog ,
            Row<Idle                , IInviteAcceptEvent           , Established         , SignallingOngoing                 , TargetIsIncomingDialog ,
            Row<Idel                , IInviteRejectAcknowledgeEvent, Terminated          , none                              , TargetIsOutgoingDialog ,
            Row<Idle                , ITerminatingEvent            , Terminated          , none                              , none                   ,
            Row<EstablishingUnstable, IInviteAcceptEvent           , Established         , SignallingOngoing                 , TargetIsIncomingDialog ,
            Row<EstablishingUnstable, IInviteRejectAcknowledgeEvent, Terminated          , none                              , TargetIsOutgoingDialog ,
            Row<EstablishingUnstable, IByeAcceptEvent              , Terminated          , none                              , And(EventSourceIsNotService, TargetIsIncomingDialog),
            Row<EstablishingUnstable, IByeRejectEvent              , Terminated          , none                              , And(EventSourceIsNotService, TargetIsIncomingDialog),
            Row<EstablishingUnstable, ITerminatingEvent            , Terminated          , none                              , none                   ,
            Row<Established         , IInviteAcknowledgeEvent      , Established         , FromSignallingOngoingToEstablished, SameCSeqAsIntialInvite ,
            Row<Established         , IByeEvent                    , TerminatingUnstable , FromEstablishedToTerminating      , EventSourceIsNotService,
            Row<Established         , ITerminatingEvent            , Terminated          , FromTerminatingToFinal            , none                   ,
            Row<TerminatingUnstable , IByeAcceptEvent              , Terminated          , none                              , none                   ,
            Row<TerminatingUnstable , IByeRejectEvent              , Terminated          , none                              , none                   ,
            Row<TerminatingUnstable , ITerminatingEvent            , Terminated          , FromTerminatingToFinal            , none                   
                                 > {};
    };

    typedef lbl::StateMachine<StateMachine> SessionState;

    XXXService() {
        mState.start()
    }

    void handleEvent(IEvent & iEvent)
    {
        //Check event
        switch (iEvent.getEventID())
        {
            case IEvent::INVITE:
                mState.processEvent(static_cast<IInviteEvent &>(iEvent));
                break;
            case IEvent::PROVISIONAL_RESPONSE:
                mState.processEvent(static_cast<IProvisionalResponseEvent &>(iEvent));
                break;
            case IEvent::INVITE_ACCEPT:
                mState.processEvent(static_cast<IInviteAcceptEvent &>(iEvent));
                break;
            case IEvent::INVITE_ACKNOWLEDGE:
                mState.processEvent(static_cast<IInviteAcknowledgeEvent &>(iEvent));
                break;
            case IEvent::INVITE_REJECT_ACKNOWLEDGE:
                mState.processEvent(static_cast<IInviteRejectAcknowledgeEvent &>(iEvent));
                break;
            case IEvent::BYE:
                mState.processEvent(static_cast<IByeEvent &>(iEvent));
                break;
            case IEvent::BYE_ACCEPT:
                mState.processEvent(static_cast<IByeAcceptEvent&>(iEvent));
                break;
            case IEvent::BYE_REJECT:
                mState.processEvent(static_cast<IByeRejectEvent&>(iEvent));
                break;
            case IEvent::TERMINATE_SESSION:
                mState.processEvent(static_cast<ITerminatingEvent&>(iEvent));
                break;
            default:
                break;
        }
    }

private:
    SessionState mState;
};

int main()
{
}
