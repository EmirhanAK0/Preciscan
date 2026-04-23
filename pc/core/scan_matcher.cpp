/**
 * pc/core/scan_matcher.cpp
 * 
 * Implementation of ScanMatcher.
 * FIFO-based dual-stream trigger + profile matching.
 */

#include "scan_matcher.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <climits>

// Placeholder for LaserProfile (assumed to be defined elsewhere)
// For now, assume it has: uint64_t timestamp_us, and other fields
struct LaserProfile {
    uint64_t timestamp_us = 0;
    uint32_t session_id = 0;
    // ... other fields (point cloud, etc.)
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

ScanMatcher::ScanMatcher(uint32_t timeout_us)
    : m_timeout_us(timeout_us)
{
}

ScanMatcher::~ScanMatcher()
{
    if (m_matcher_thread.joinable()) {
        m_matcher_thread.join();
    }
}

// ============================================================================
// Public API
// ============================================================================

void ScanMatcher::addTrigger(const TriggerEvent& evt)
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    m_trigger_queue.push(evt);
}

void ScanMatcher::addProfile(const LaserProfile& prof)
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    m_profile_queue.push(prof);
}

bool ScanMatcher::tryGetMatched(MatchedScanFrame& frame, int timeout_ms)
{
    std::unique_lock<std::mutex> lock(m_output_mutex);
    
    if (m_output_queue.empty()) {
        return false;  // Queue empty
    }
    
    frame = m_output_queue.front();
    m_output_queue.pop();
    return true;
}

size_t ScanMatcher::triggerQueueSize() const
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_trigger_queue.size();
}

size_t ScanMatcher::profileQueueSize() const
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_profile_queue.size();
}

ScanMatcher::Stats ScanMatcher::getStats() const
{
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    return m_stats;
}

void ScanMatcher::clearStats()
{
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_stats = Stats();
}

// ============================================================================
// Matching Logic
// ============================================================================

/**
 * Try to match one trigger-profile pair.
 * Implements FIFO matching with timeout and latency check.
 * 
 * Algorithm:
 * 1. If both queues have items:
 *    a. Compute latency = profile.timestamp_us - trigger.tick_us
 *    b. If latency < 0: profile is old, drop profile, return (requeue trigger)
 *    c. If latency > timeout: trigger is old, drop trigger, return (requeue profile)
 *    d. Else: match and emit
 * 2. If only trigger queue has items: wait (profile might arrive)
 * 3. If only profile queue has items: wait (trigger might arrive)
 * 4. If both empty: return
 */
bool ScanMatcher::tryMatchOne(MatchedScanFrame& frame)
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    
    // Check if both queues have data
    if (m_trigger_queue.empty() || m_profile_queue.empty()) {
        return false;  // Can't match, need both
    }
    
    TriggerEvent trigger = m_trigger_queue.front();
    LaserProfile profile = m_profile_queue.front();
    
    int64_t latency_us = (int64_t)profile.timestamp_us - (int64_t)trigger.tick_us;
    
    if (latency_us < 0) {
        // Profile arrived before trigger; profile is old
        frame.status = MatchedScanFrame::DROPPED_PROFILE;
        frame.profile = profile;
        frame.latency_us = latency_us;
        
        {
            std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
            m_stats.dropped_profiles++;
        }
        
        m_profile_queue.pop();
        return true;
    }
    
    if (latency_us > (int64_t)m_timeout_us) {
        // Trigger has been waiting too long; trigger is orphan
        frame.status = MatchedScanFrame::ORPHAN_TRIGGER;
        frame.trigger = trigger;
        frame.latency_us = latency_us;
        
        {
            std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
            m_stats.orphan_triggers++;
        }
        
        m_trigger_queue.pop();
        return true;
    }
    
    // Latency is within acceptable range; MATCH!
    frame.status = MatchedScanFrame::MATCHED;
    frame.trigger = trigger;
    frame.profile = profile;
    frame.latency_us = latency_us;
    
    {
        std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
        m_stats.matched++;
        m_stats.min_latency_us = std::min(m_stats.min_latency_us, latency_us);
        m_stats.max_latency_us = std::max(m_stats.max_latency_us, latency_us);
        m_stats.sum_latency_us += latency_us;
        m_stats.latency_samples++;
    }
    
    m_trigger_queue.pop();
    m_profile_queue.pop();
    return true;
}

// ============================================================================
// Matcher Thread
// ============================================================================

void ScanMatcher::matcherThreadMain()
{
    std::cout << "ScanMatcher: matcher thread started" << std::endl;
    
    while (m_running.load()) {
        MatchedScanFrame frame;
        
        // Try to match one pair
        if (tryMatchOne(frame)) {
            // Add to output queue
            {
                std::lock_guard<std::mutex> lock(m_output_mutex);
                if (m_output_queue.size() < MAX_OUTPUT_QUEUE) {
                    m_output_queue.push(frame);
                } else {
                    std::cerr << "ScanMatcher: output queue overflow" << std::endl;
                }
            }
        } else {
            // No match available; sleep a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    std::cout << "ScanMatcher: matcher thread exiting" << std::endl;
}

// ============================================================================
// Thread Management (if needed in future)
// ============================================================================

// For now, matching is done on-demand in tryGetMatched.
// If we add a background matcher thread, uncomment:
/*
bool ScanMatcher::start()
{
    if (m_running.load()) {
        return true;
    }
    
    m_running = true;
    try {
        m_matcher_thread = std::thread(&ScanMatcher::matcherThreadMain, this);
    } catch (const std::exception& e) {
        std::cerr << "ScanMatcher: failed to start thread: " << e.what() << std::endl;
        m_running = false;
        return false;
    }
    
    return true;
}

bool ScanMatcher::stop()
{
    if (!m_running.load()) {
        return true;
    }
    
    m_running = false;
    if (m_matcher_thread.joinable()) {
        m_matcher_thread.join();
    }
    
    return true;
}
*/

// ============================================================================
// End of Implementation
// ============================================================================
