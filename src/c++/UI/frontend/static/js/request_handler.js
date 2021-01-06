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

/**
 * @brief Perform a GET request on the desired page and return the result
 * @param {String} url The url to GET the data at
 * @returns {JSON} Contains the backend's json response to the GET request
 */
export async function getJsonData(url) {
    let reqResponse = {}
    try {
        reqResponse = await $.ajax({
            url: url,
            type: 'GET',
            // dataType: "json", // return from web app server might be text -- triggers error
            contentType: "application/json",
        })
    } catch (err) {
        console.log(`Failed to GET data from '${curr_page}': ${err.status} - ${err.statusText}`);
    }
    return reqResponse
}

/**
 * @brief Perform a GET/POST request on the desired page and check if it exists
 * @param {String} url The url to GET/POST
 * @param {"POST", "GET"} method The type of request to perform 
 * @returns {Boolean} True if page exists and is requestable. False if error/dne 
 */
export async function doesPageExist(url, method) {
    let reqResponse = {}
    try {
        reqResponse = await $.ajax({
            url: url,
            type: method,
            timeout: 1000 // set timeout to 1 sec
        })
        return true
    } catch (err) {
        return false
    }
}
