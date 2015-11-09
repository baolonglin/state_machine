
#include "fsm.h"

class IEvent
{
public:
    enum EventID {
        DEFAULT = 0,
        TERMINATE_SESSION,
        SERVICE_REFRESH,
        TERMINATE_DIALOG,
        CREATE_DIALOG,
        BYE,
        BYE_ACCEPT,
        BYE_REJECT,
        CANCEL,
        CANCEL_ACCEPT,
        CANCEL_REJECT,              //10
        INVITE,
        INVITE_ACCEPT,
        INVITE_ACKNOWLEDGE,
        INVITE_REJECT,
        INVITE_REJECT_ACKNOWLEDGE,
        PROVISIONAL_RESPONSE,
        CANCEL_REINVITE,
        CANCEL_REINVITE_ACCEPT,
        CANCEL_REINVITE_REJECT,
        REINVITE,                   //20
        REINVITE_ACCEPT,
        REINVITE_ACKNOWLEDGE,
        REINVITE_REJECT,
        REINVITE_REJECT_ACKNOWLEDGE,
        UPDATE,
        EARLY_UPDATE,
        UPDATE_ACCEPT,
        EARLY_UPDATE_ACCEPT,
        EARLY_UPDATE_REJECT,
        UPDATE_REJECT,              //30
        REGISTER,
        REGISTER_ACCEPT,
        REGISTER_REJECT,
        PRACK,
        PRACK_ACCEPT,
        PRACK_REJECT,
        REFER,
        REFER_ACCEPT,
        REFER_REJECT,
        NOTIFY,                     //40
        NOTIFY_ACCEPT,
        NOTIFY_REJECT,
        OPTIONS,
        OPTIONS_ACCEPT,
        OPTIONS_REJECT,
        MESSAGE,
        MESSAGE_ACCEPT,
        MESSAGE_REJECT,
        SUBSCRIBE,
        SUBSCRIBE_ACCEPT,           //50
        SUBSCRIBE_REJECT,
        GENERATE_CALL,
        INFO,
        INFO_ACCEPT,
        INFO_REJECT,
        INVITE_ACCEPT_EARLY_NOTIFICATION,
        PUBLISH,
        PUBLISH_ACCEPT,
        PUBLISH_REJECT,
        PX_MAKE_CALL_SESSION,      // 60
        PX_GET_CALL_SESSION_INFO_REQ,
        PX_GET_CALL_SESSION_INFO_RESP,
        PX_END_CALL_SESSION,
        PX_HANDLE_CALLED_NUMBER,
        PX_HANDLE_CALLED_NUMBER_ACCEPT,
        PX_HANDLE_CALLED_NUMBER_REJECT,
        TERMINATE_WITH_ANNOUNCEMENT,
        DIALOG_CREATED,
        DIALOG_TERMINATED,
        CCMP_KICKOUT,              // 70
        CCMP_KICKOUT_ACCEPT,
        CCMP_KICKOUT_REJECT,
        CCMP_MEDIASTREAM_PROPERTY_UPDATE,
        CCMP_MEDIASTREAM_PROPERTY_UPDATE_ACCEPT,
        CCMP_MEDIASTREAM_PROPERTY_UPDATE_REJECT,
        CCMP_DELETE_CONFERENCE,
        CCMP_DELETE_CONFERENCE_ACCEPT,
        CCMP_DELETE_CONFERENCE_REJECT,
        CCMP_DIALOUT,
        CCMP_DIALOUT_ACCEPT,
        CCMP_DIALOUT_REJECT,
        CCMP_LOCK,
        CCMP_LOCK_ACCEPT,
        CCMP_LOCK_REJECT,

        EVENT_MAX
    };

    EventID getEventID() const {
        return mEventID;
    }
private:
    const EventID mEventID;
};

struct EventCompare : public std::binary_function<IEvent::EventID, IEvent::EventID, bool>
{
    bool operator()(const IEvent::EventID& lhs, const IEvent::EventID& rhs) const
    {
        return lhs < rhs;
    }
};

class XXXService
{
public:
    XXXService() :
        mStateMachine(this, &mIdleState)
    {
        mIdleState.add(IEvent::INVITE              , mIdleState                , &XXXService::handleInvite);
        mIdleState.add(IEvent::PROVISIONAL_RESPONSE, mEstablishingUnstableState);
    }

    void handleInvite(const IEvent& ev)
    {
    }
private:
    typedef lbl::State<XXXService, IEvent, EventCompare> S;
    S mIdleState;
    S mEstablishingUnstableState;
    S mEstablishedState;
    S mTerminatingUnstableState;
    S mTerminatedState; 

    typedef lbl::StateMachine<XXXService, IEvent, EventCompare> SM;
    SM mStateMachine;
};

int main()
{
    XXXService service;
    return 0;
}
