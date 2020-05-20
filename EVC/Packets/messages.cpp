#include "messages.h"
#include "information.h"
#include "12.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "136.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Procedures/override.h"
#include "../TrainSubsystems/brake.h"
static int reading_nid_bg=-1;
static int reading_nid_c=-1;
static std::vector<eurobalise_telegram> telegrams;
static distance last_passed_distance;
static bool reffound=false;
static bool refmissed=false;
static bool dupfound=false;
static bool refpassed=false;
static bool linked = false;
static int prevpig=-1;
static int totalbg=8;
static bool reading = false;
static int dir = -1;
static int64_t first_balise_time;
static distance bg_reference1;
static distance bg_reference1max;
static distance bg_reference1min;
static distance bg_reference;
static distance bg_referencemax;
static distance bg_referencemin;
static bool stop_checking_linking=false;
std::list<link_data>::iterator link_expected=linking.end();
std::deque<eurobalise_telegram> pending_telegrams;
void trigger_reaction(int reaction);
void handle_telegrams(std::vector<eurobalise_telegram> message, distance dist, int dir, int64_t timestamp, bg_id nid_bg);
void check_valid_data(std::vector<eurobalise_telegram> telegrams, distance bg_reference, bool linked, int64_t timestamp);
void check_eurobalise_passed();
void balise_group_passed();
void check_linking();
void expect_next_linking();
void trigger_reaction(int reaction)
{
    switch (reaction) {
        case 1:
            trigger_brake_reason(0);
            break;
        case 2:
            //No reaction
            break;
        default:
            //Train Trip - Balise read error - Linking"
            trigger_condition(17);
            break;
    }
}
void reset_eurobalise_data()
{
    telegrams.clear();
    reading_nid_bg = -1;
    reading_nid_c=-1;
    prevpig = -1;
    totalbg = 8;
    reading = false;
    dir = -1;
    refmissed = false;
    reffound = false;
    dupfound = false;
    refpassed = true;
    linked = false;
    stop_checking_linking = false;
}
void expect_next_linking()
{
    if (link_expected != linking.end())
        ++link_expected;
}
void check_linking()
{
    if (link_expected!=linking.end() && !stop_checking_linking) {
        auto link_bg=linking.end();
        for (auto it = link_expected; it!=linking.end(); ++it) {
            if (it->nid_bg == bg_id({reading_nid_c, reading_nid_bg})) {
                link_bg = it;
                break;
            }
        }
        bool isexpected = linked && refpassed && link_expected==link_bg;
        bool c1 = isexpected && link_expected->min() > bg_referencemax;
        bool c2 = link_expected->max() < d_minsafefront(0);
        bool c3 = linked && link_bg!=linking.end() && link_bg != link_expected;
        if (c1 || c2 || c3) {
            trigger_reaction(link_expected->reaction);
            expect_next_linking();
            if (c3)
                check_linking();
            else if (c1)
                stop_checking_linking = true;
        }
        else if (isexpected) {
            stop_checking_linking = true;
            if (bg_referencemax>=link_expected->min() && bg_referencemin<=link_expected->max())
                expect_next_linking();
        }
    }
}
void balise_group_passed()
{
    if (!reffound) {
        refmissed = true;
        if (dupfound) {
            bg_reference = bg_reference1;
            bg_referencemax = bg_reference1max;
            bg_referencemin = bg_reference1min;
            refpassed = true;
            check_linking();
        }
    }
    bool linking_rejected=false;
    if (linked && reffound && !linking.empty()) {
        linking_rejected = true;
        for (link_data l : linking) {
            if (l.nid_bg == bg_id({reading_nid_c, reading_nid_bg})) {
                if (dir != -1 && dir != l.reverse_dir)
                    trigger_condition(66);
                if (l.max() >= bg_referencemin && l.min() <= bg_referencemax)
                    linking_rejected = false;
                break;
            }
        }
    }
    if (!linking_rejected) check_valid_data(telegrams, bg_reference, linked, first_balise_time);
    linking.erase(linking.begin(), link_expected);
    reset_eurobalise_data();
}
void check_eurobalise_passed()
{
    check_linking();
    if (reading) {
        double elapsed = d_estfront-last_passed_distance;
        if (prevpig==-1) {
            if (elapsed > 12*8)
                balise_group_passed();
        } else {
            int v1 = (totalbg-prevpig-1);
            int v2 = prevpig;
            if (dir == 0 && elapsed > v1*12)
                balise_group_passed();
            else if (dir == 1 && elapsed > v2*12)
                balise_group_passed();
            else if (dir == -1 && elapsed > std::max(v1, v2)*12)
                balise_group_passed();
        }
    }
    if (pending_telegrams.empty()) return;
    eurobalise_telegram t = pending_telegrams.front();
    pending_telegrams.pop_front();
    last_passed_distance = d_estfront;
    reading = true;
    if (!t.readerror) {
        linked = t.Q_LINK == Q_LINK_t::Linked;
        if (reading_nid_bg != -1 && reading_nid_bg != t.NID_BG)
            balise_group_passed();
        if (reading_nid_bg != t.NID_BG) {
            first_balise_time = get_milliseconds();
        }
        reading_nid_bg = t.NID_BG;
        reading_nid_c = t.NID_C;
        totalbg = t.N_TOTAL+1;
        if (prevpig != -1) {
            if (dir == -1)
                dir = (prevpig>t.N_PIG) ? 1 : 0;
            if (t.N_PIG>prevpig && !reffound)
                refmissed = true;
        }
        prevpig = t.N_PIG;
        if (t.N_PIG == 1 && t.M_DUP == M_DUP_t::DuplicateOfPrev) {
            dupfound = true;
            bg_reference1 = d_estfront;
            bg_reference1max = d_maxsafefront(0);
            bg_reference1min = d_minsafefront(0);
        }
        if (t.N_PIG == 0) {
            reffound = true;
            bg_reference = d_estfront;
            bg_referencemax = d_maxsafefront(0);
            bg_referencemin = d_minsafefront(0);
            refpassed = true;
        }
        if ((dir==0 && t.N_PIG == t.N_TOTAL) || (dir == 1 && t.N_PIG == 0)) {
            telegrams.push_back(t);
            balise_group_passed();
            return;
        }
    }
    telegrams.push_back(t);
    if (refmissed && dupfound) {
        bg_reference = bg_reference1;
        bg_referencemax = bg_reference1max;
        bg_referencemin = bg_reference1min;
        refpassed = true;
    }
}
void check_valid_data(std::vector<eurobalise_telegram> telegrams, distance bg_reference, bool linked, int64_t timestamp)
{
    int nid_bg=-1;
    int nid_c=-1;
    int dir=-1;
    int prevno=-1;
    std::vector<eurobalise_telegram> read_telegrams;
    for (int i=0; i<telegrams.size(); i++) {
        eurobalise_telegram t = telegrams[i];
        if (!t.readerror) {
            nid_bg = t.NID_BG;
            nid_c = t.NID_C;
            if (prevno == -1)
                prevno = t.N_PIG;
            else
                dir = t.N_PIG<prevno; 
            read_telegrams.push_back(t);
        }
    }
    if (dir == 1)
        std::reverse(read_telegrams.begin(), read_telegrams.end());

    bool accepted1 = true;
    if (read_telegrams.size() == 0)
        accepted1 = false;

    link_data balise_link;
    bool containedinlinking=false;
    if (linked && !linking.empty()) {
        for (link_data l : linking) {
            if (l.nid_bg == bg_id({nid_c, nid_bg})) {
                containedinlinking = true;
                balise_link = l;
            }
        }
        if (!containedinlinking)
            return;
    }

    std::set<int> missed;
    for (int i=0; i<read_telegrams.size(); i++) {
        eurobalise_telegram t = read_telegrams[i];
        if (i==0 && t.N_PIG != 0) {
            for (int j=0; j<t.N_PIG; j++)
                missed.insert(j);
        }
        if ((i>1 && telegrams[i-1].N_PIG+1<t.N_PIG)) {
            for (int j=telegrams[i-1].N_PIG; j<t.N_PIG; j++)
                missed.insert(j);
        }
        if (i+1==read_telegrams.size() && t.N_PIG!=t.N_TOTAL) {
            for (int j=t.N_PIG; j<=t.N_TOTAL; j++)
                missed.insert(j);
        }
    }
    for (int pig : missed) {
        bool reject=true;
        for (int i=0; i<read_telegrams.size(); i++) {
            eurobalise_telegram t = read_telegrams[i];
            if ((t.M_DUP == M_DUP_t::DuplicateOfNext && t.N_PIG+1==pig) || (t.M_DUP == M_DUP_t::DuplicateOfPrev && t.N_PIG==pig+1)) {
                if (containedinlinking) {
                    reject = false;
                } else {
                    if (dir!=-1) {
                        reject = false;
                    } else {
                        bool directional = false;
                        for (int j=0; j<t.packets.size(); j++) {
                            ETCS_packet *p = t.packets[i].get();
                            if (p->directional && ((ETCS_directional_packet*)p)->Q_DIR != Q_DIR_t::Both)
                                directional = true;
                        }
                        if (!directional)
                            reject = false;
                    }
                }
                break;
            }
        }
        if (reject)
            accepted1 = false;
    }
    std::vector<eurobalise_telegram> message;
    for (int i=0; i<read_telegrams.size(); i++) {
        eurobalise_telegram t = read_telegrams[i];
        bool c1 = t.M_DUP == M_DUP_t::NoDuplicates;
        bool c2 = t.M_DUP == M_DUP_t::DuplicateOfNext && i+1<read_telegrams.size() && t.N_PIG+1==read_telegrams[i+1].N_PIG;
        bool c3 = t.M_DUP == M_DUP_t::DuplicateOfPrev && i>1 && t.N_PIG==read_telegrams[i-1].N_PIG+1;
        if (c1 || !(c2||c3))
            message.push_back(t);
        if ((c2 && dir==0) || (c3 && dir==1)) {
            eurobalise_telegram first = t;
            eurobalise_telegram second = c2 ? read_telegrams[i+1] : read_telegrams[i-1];
            bool seconddefault = false;
            for (int j=0; j<second.packets.size(); j++) {
                if (second.packets[j]->NID_PACKET == 254) {
                    seconddefault = true;
                    break;
                }
            }
            message.push_back(seconddefault ? first : second);
        }
    }
    bool accepted2=true;
    int mcount=-1;
    for (int i=0; i<message.size(); i++) {
        if (!message[i].valid)
            accepted2 = false;
        if (message[i].M_MCOUNT==M_MCOUNT_t::NeverFitsTelegrams) {
            accepted2 = false;
        } else if(message[i].M_MCOUNT!=M_MCOUNT_t::FitsAllTelegrams) {
            if (mcount==-1)
                mcount = message[i].M_MCOUNT;
            else if (mcount != message[i].M_MCOUNT)
                accepted2 = false;
        }
    }
    bool accepted = accepted1 && accepted2;
    if (!accepted) {
        if (containedinlinking) {
            trigger_reaction(balise_link.reaction);
        } else {
            if (accepted2) {
                for (int i=0; i<message.size(); i++) {
                    eurobalise_telegram t = message[i];
                    if (t.packets.empty())
                        continue;
                    for (int j=0; j<t.packets.size()-1; j++) {
                        auto p = t.packets[j];
                        if (p->NID_PACKET == 145) {
                            return;
                        }
                    }
                }
            }
            trigger_reaction(1);
        }
        return;
    }
    
    bg_reference = distance(0,update_location_reference({nid_c, nid_bg}, bg_reference, linked));
    if (dir == -1 && containedinlinking)
        dir = balise_link.reverse_dir;
    handle_telegrams(message, bg_reference, dir, timestamp, {nid_c, nid_bg});
    if (dir != -1)
        geographical_position_handle_bg_passed({nid_c, nid_bg}, bg_reference, dir == 1);
}
void handle_telegrams(std::vector<eurobalise_telegram> message, distance dist, int dir, int64_t timestamp, bg_id nid_bg)
{
    if (!ongoing_transition) {
        transition_buffer.clear();
    } else {
        if (transition_buffer.size() == 3)
            transition_buffer.pop_front();
        transition_buffer.push_back({});
    }

    std::list<std::shared_ptr<etcs_information>> ordered_info;
    for (int i=0; i<message.size(); i++) {
        eurobalise_telegram t = message[i];
        distance ref = dist;
        bool infill=false;
        for (int j=0; j<t.packets.size(); j++) {
            ETCS_packet *p = t.packets[j].get();
            if (p->directional) {
                auto *dp = (ETCS_directional_packet*)p;
                if (dir == -1 || (dp->Q_DIR == Q_DIR_t::Nominal && dir == 1) || (dp->Q_DIR == Q_DIR_t::Reverse && dir == 0))
                    continue;
            }
            if (p->NID_PACKET == 136) {
                InfillLocationReference ilr = *((InfillLocationReference*)p);
                bool found = false;
                for (link_data l : linking) {
                    if (l.nid_bg == bg_id({ilr.Q_NEWCOUNTRY == Q_NEWCOUNTRY_t::SameCountry ? nid_bg.NID_C : ilr.NID_C, ilr.NID_BG})) {
                        found = true;
                        infill = true;
                        ref = l.dist;
                        break;
                    }
                }
                if (!found)
                    break;
            }
            std::vector<etcs_information*> info = construct_information(p->NID_PACKET);
            for (int i=0; i<info.size(); i++) {
                info[i]->linked_packets.push_back(t.packets[j]);
                info[i]->ref = ref;
                info[i]->infill = infill;
                info[i]->dir = dir;
                info[i]->fromRBC = false;
                info[i]->timestamp = timestamp;
                info[i]->nid_bg = nid_bg;
                if (info[i]->index == 3) {
                    ordered_info.push_back(std::shared_ptr<etcs_information>(info[i]));
                } else {
                    ordered_info.push_front(std::shared_ptr<etcs_information>(info[i]));
                }
            }
        }
    }
    for (auto it = ordered_info.begin(); it!=ordered_info.end(); ++it)
    {
        try_handle_information(*it, ordered_info);
    }
}
struct level_filter_data
{
    int num;
    Level level;
    bool fromRBC;
    bool operator<(const level_filter_data &o) const
    {
        if (num==o.num) {
            if (level == o.level)
                return fromRBC<o.fromRBC;
            return level<o.level;
        }
        return num<o.num;
    }
};
struct accepted_condition
{
    bool reject;
    std::set<int> exceptions;
};
std::map<level_filter_data, accepted_condition> level_filter_index;
bool level_filter(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message) 
{
    accepted_condition s = level_filter_index[{info->index, level, info->fromRBC}];
    if (!s.reject) {
        if (s.exceptions.find(3) != s.exceptions.end()) {
            //TODO: RBC dependent
        }
        if (s.exceptions.find(4) != s.exceptions.end()) {
            if (info->linked_packets.begin()->get()->NID_PACKET == 12) {
                Level1_MA ma = *((Level1_MA*)info->linked_packets.begin()->get());
                movement_authority MA = movement_authority(info->ref, ma, info->timestamp);
                distance end = MA.get_abs_end();
                if (get_SSP().empty() || (--get_SSP().end())->get_end()<end || get_SSP().begin()->get_start() > d_estfront)
                    return false;
                if (get_gradient().empty() || (--get_gradient().end())->first<end || get_gradient().begin()->first > d_estfront)
                    return false;   
            }
        }
        if (s.exceptions.find(9) != s.exceptions.end()) {
            if (!ongoing_transition || (ongoing_transition->leveldata.level != Level::N2 && ongoing_transition->leveldata.level != Level::N3))
                return false;
        }
        if (s.exceptions.find(11) != s.exceptions.end()) {
            if (ongoing_transition)
                return false;
        }
        if (s.exceptions.find(7) != s.exceptions.end()) {
            //TODO: NTC dependent
        }
        return true;
    } else {
        if (s.exceptions.find(1) != s.exceptions.end()) {
            if (ongoing_transition && ongoing_transition->leveldata.level == Level::N1)
                transition_buffer.back().push_back(info);
            return false;
        }
        if (s.exceptions.find(2) != s.exceptions.end()) {
            if (ongoing_transition && (ongoing_transition->leveldata.level == Level::N2 || ongoing_transition->leveldata.level == Level::N3))
                transition_buffer.back().push_back(info);
            return false;
        }
        if (s.exceptions.find(6) != s.exceptions.end()) {
            /*if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc == nid_ntc)
                transition_buffer.back().insert(info);*/
            return false;
        }
        return false;
    }
    return false;
}
void set_level_filter()
{
    std::vector<std::vector<std::string>> conds = {
        {"A","A","A","A","A","R2","R2","R2","A","A"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","","","","",""},
        {"R1","R1","A4","R1","R1","R2","R2","R2","A3,4,5","A3,4,5"},
        {"R","R","A","R","R","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"A11","A11","A11","A11","A11","","","","",""},
        {"A","A","A","A14","A14","A","A","A","A","A"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"","","","","","A","A","A","A","A"},
        {"","","","","","A","A","A","A","A"},
        {"","","","","","R","R","R","A3","A3"},
        {"R","R","A","A","A","","","","",""},
        {"R","R","A","R","R","","","","",""},
        {"A","R1,2","A","A8","A8","R2","R2","R2","A3","A3"},
        {"A","R1,2","A","A","A","R2","R2","R2","A3","A3"},
        {"","","","","","R2","R2","R2","A","A"},
        {"A","R1,2","A","A","A","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R","R","R2","R2","R2","A","A"},
        {"A","R1,2","A","A","A","R2","R2","R2","A12","A12"},
        {"A","R1,2","A","A","A","R2","R2","R2","A12","A12"},
        {"A","R1,2","A","A","A","R2","R2","R2","A","A"},
        {"R","R","R","A","A","R","R","R","A3","A3"},
        {"A13","A13","A","A","A","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"R","R","A","R1","R1","","","","",""},
        {"R","R","A","R","R","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","A10","A10","A10","A10","A10"},
        {"R","R","A","R1","R1","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","R","R","R","A","A"},
        {"","","","","","A","A","A","A","A"},
        {"","","","","","R","R","R","A3,4,5","A3,4,5"},
        {"","","","","","R2","R2","R2","A","A"},
        {"","","","","","R2","R2","R2","A","A"},
        {"","","","","","R","R","R","A3","A3"},
        {"","","","","","R","R","R","A3","A3"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","R","R","R","A3","A3"},
        {"","","","","","R","R","R","A3","A3"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"","","","","","R","R","R","A","A"},
        {"","","","","","R","R","R","A","A"},
        {"","","","","","R","R","R","A","A"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","","","","",""},
        {"A9","A9","A9","R","R","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1,2","R1,2","A","A","A","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3,5","A3,5"},
        {"R","R","A","R","R","R","R","R","A","A"},
        {"A","A","A","A","A","A","A","A","A","A"}
    };
    Level levels[] = {Level::N0, Level::NTC, Level::N1, Level::N2, Level::N3};
    for (int i=0; i<conds.size(); i++) {
        for (int j=0; j<10; j++) {
            std::string str = conds[i][j];
            if (str != "") {
                bool rej = str[0] == 'R';
                str = str.substr(1);
                std::set<int> except;
                for(;!str.empty();) {
                    std::size_t index = str.find_first_of(',');
                    except.insert(std::stoi(str.substr(0, index)));
                    if (index != std::string::npos)
                        str = str.substr(index+1);
                    else
                        break;
                }
                level_filter_index[{i, levels[j%5], j>4}] = {rej, except};
            }
        }
    }
}
struct mode_filter_data
{
    int num;
    Mode mode;
    bool operator<(const mode_filter_data &o) const
    {
        if (num==o.num) {
            return mode<o.mode;
        }
        return num<o.num;
    }
};
std::map<mode_filter_data, accepted_condition> mode_filter_index;
void set_mode_filter()
{
    /*std::vector<std::vector<std::string>> conds = {
    };
    Mode modes[] = {};
    for (int i=0; i<conds.size(); i++) {
        for (int j=0; j<17; j++) {
            std::string str = conds[i][j];
            if (str != "" && str != "NR") {
                bool rej = str[0] == 'R';
                str = str.substr(1);
                std::set<int> except;
                for(;!str.empty();) {
                    std::size_t index = str.find_first_of(',');
                    except.insert(std::stoi(str.substr(0, index)));
                    if (index != std::string::npos)
                        str = str.substr(index+1);
                    else
                        break;
                }
                mode_filter_index[{i, modes[j]}] = {rej, except};
            }
        }
    }*/
}
void set_message_filters()
{
    set_mode_filter();
    set_level_filter();
}
bool second_filter(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message)
{
    if (!info->fromRBC)
        return true;
    return false;
}
bool mode_filter(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message)
{
    /*accepted_condition s = mode_filter_index[{info->index, mode}];
    return !s.reject;*/
    return true;
}
void try_handle_information(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message)
{
    if (!level_filter(info, message)) return;
    if (!second_filter(info, message)) return;
    if (!mode_filter(info, message)) return;
    info->handle();
}
std::vector<etcs_information*> construct_information(int packet_num)
{
    std::vector<etcs_information*> info;
    if (packet_num == 5) {
        info.push_back(new linking_information());
    } else if (packet_num == 12) {
        info.push_back(new ma_information());
        info.push_back(new signalling_information());
    } else if (packet_num == 21) {
        info.push_back(new gradient_information());
    } else if (packet_num == 27) {
        info.push_back(new issp_information());
    } else if (packet_num == 41) {
        info.push_back(new leveltr_order_information());
    } else if (packet_num == 41) {
        info.push_back(new condleveltr_order_information());
    } else if (packet_num == 68) {
        info.push_back(new track_condition_information());
    } else if (packet_num == 72) {
        info.push_back(new plain_text_information());
    } else if (packet_num == 79) {
        info.push_back(new geographical_position_information());
    } else if (packet_num == 137) {
        info.push_back(new stop_if_in_SR_information());
    }
    return info;
}