'use strict';
/**
 * @file Handles the POST/GET requests
 */

/**
 * @brief Sends a post request to the current page with updated packet
 * @param {JSON} pkt_data The data to send to the web app
 * @returns {reqResponse} Contains the backend's json response to post request
 */
export async function postPktData(pkt_data) {
    const curr_page = window.location.pathname
    let reqResponse = {}
    try {
        reqResponse = await $.ajax({
            url: curr_page,
            type: 'POST',
            // dataType: "json", // return from web app server might be text -- triggers error
            contentType: "application/json",
            data: JSON.stringify(pkt_data),
        })
    } catch (err) {
        console.log(`Failed to post to '${curr_page}': ${err.status} - ${err.statusText}`);
    }
    return reqResponse
}
