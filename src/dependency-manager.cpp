
#include "dependency-manager.hpp"

void full_dependency_manager_t::node_changed(osmid_t id) {
    for (auto const way_id : m_object_store->get_ways_by_node(id)) {
        way_changed(way_id);
        m_ways_pending_tracker.mark(way_id);
    }

    for (auto const rel_id : m_object_store->get_rels_by_node(id)) {
        m_rels_pending_tracker.mark(rel_id);
    }
}

void full_dependency_manager_t::way_changed(osmid_t id) {
    if (m_ways_pending_tracker.is_marked(id)) {
        return;
    }

    for (auto const rel_id : m_object_store->get_rels_by_way(id)) {
        m_rels_pending_tracker.mark(rel_id);
    }
}

void full_dependency_manager_t::relation_changed(osmid_t id) {
    if (m_rels_pending_tracker.is_marked(id)) {
        return;
    }

    for (auto const rel_id : m_object_store->get_rels_by_rel(id)) {
        m_rels_pending_tracker.mark(rel_id);
    }
}

void full_dependency_manager_t::relation_deleted(osmid_t id) {
    for (auto const rel_id : m_object_store->get_ways_by_rel(id)) {
        m_ways_pending_tracker.mark(rel_id);
    }
}

bool full_dependency_manager_t::has_pending() const noexcept
{
    return !m_ways_pending_tracker.empty() || !m_rels_pending_tracker.empty();
}

void full_dependency_manager_t::process_pending(middle_t::pending_processor &pf)
{
    osmid_t id;
    while (id_tracker::is_valid(id = m_ways_pending_tracker.pop_mark())) {
        pf.enqueue_ways(id);
    }

    pf.process_ways();

    while (id_tracker::is_valid(id = m_rels_pending_tracker.pop_mark())) {
        pf.enqueue_relations(id);
    }

    pf.process_relations();
}
