#pragma once
#include "etcs_information.h"
#include "messages.h"
#include "12.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "68.h"
#include "72.h"
#include "79.h"
#include "136.h"
#include "137.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Procedures/override.h"
#include "../TrackConditions/track_condition.h"
#include "../DMI/text_message.h"
#include "../Position/geographical.h"
struct linking_information : etcs_information
{
    linking_information() : etcs_information(0) {}
    void handle() override
    {
        Linking l = *(Linking*)linked_packets.front().get();
        update_linking(ref, l, infill, nid_bg);
    }
};
struct signalling_information : etcs_information
{
    signalling_information() : etcs_information(2) {}
    void handle() override
    {
        Level1_MA ma = *(Level1_MA*)linked_packets.front().get();
        if (ma.V_MAIN == 0) {
            if (!overrideProcedure && !infill)
                trigger_condition(18);
        } else {
            movement_authority newMA = movement_authority(ref, ma, timestamp);
            set_signalling_restriction(newMA, infill);
        }
    }
};
struct ma_information : etcs_information
{
    ma_information() : etcs_information(3) {}
    void handle() override
    {
        Level1_MA ma = *(Level1_MA*)linked_packets.front().get();;
        movement_authority MA = movement_authority(ref, ma, timestamp);
        if (infill)
            MA_infill(MA);
        else
            replace_MA(MA);
    }
};
struct gradient_information : etcs_information
{
    gradient_information() : etcs_information(5) {}
    void handle() override
    {
        GradientProfile grad = *(GradientProfile*)linked_packets.front().get();
        std::map<distance, double> gradient;
        std::vector<GradientElement> elements;
        elements.push_back(grad.element);
        elements.insert(elements.end(), grad.elements.begin(), grad.elements.end());
        distance d = ref;
        for (auto e : elements) {
            gradient[d+e.D_GRADIENT.get_value(grad.Q_SCALE)] = (e.Q_GDIR == Q_GDIR_t::Uphill ? 1 : -1)*e.G_A;
            d += e.D_GRADIENT;
        }
        update_gradient(gradient);
    }
};
struct issp_information : etcs_information
{
    issp_information() : etcs_information(6) {}
    void handle() override
    {
        InternationalSSP issp = *(InternationalSSP*)linked_packets.front().get();
        update_SSP(get_SSP(ref, issp));
    }
};
struct leveltr_order_information : etcs_information
{
    leveltr_order_information() : etcs_information(8) {}
    void handle() override
    {
        LevelTransitionOrder LTO = *(LevelTransitionOrder*)linked_packets.front().get();
        level_transition_received(level_transition_information(LTO, ref));
    }
};
struct condleveltr_order_information : etcs_information
{
    condleveltr_order_information() : etcs_information(9) {}
    void handle() override
    {
        ConditionalLevelTransitionOrder CLTO = *(ConditionalLevelTransitionOrder*)linked_packets.front().get();
        level_transition_received(level_transition_information(CLTO, ref));
    }
};
struct stop_if_in_SR_information : etcs_information
{
    stop_if_in_SR_information() : etcs_information(15) {}
    void handle() override
    {
        StopIfInSR s = *(StopIfInSR*)linked_packets.front().get();
        // TODO: balises allowed
        if (s.Q_SRSTOP == Q_SRSTOP_t::StopIfInSR && !overrideProcedure) trigger_condition(54);
    }
};
struct plain_text_information : etcs_information
{
    plain_text_information() : etcs_information(23) {}
    void handle() override
    {
        PlainTextMessage m = *(PlainTextMessage*)linked_packets.front().get();
        add_message(m, ref);
    }
};
struct geographical_position_information : etcs_information
{
    geographical_position_information() : etcs_information(25) {}
    void handle() override
    {
        GeographicalPosition m = *(GeographicalPosition*)linked_packets.front().get();
        handle_geographical_position(m, nid_bg);
    }
};
struct track_condition_information : etcs_information
{
    track_condition_information() : etcs_information(35) {}
    void handle() override
    {
        TrackCondition tc = *(TrackCondition*)linked_packets.front().get();
        load_track_condition_various(tc, ref);
    }
};
void try_handle_information(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message);
std::vector<etcs_information*> construct_information(int packet_num);