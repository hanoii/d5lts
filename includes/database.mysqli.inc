<?php
// $Id$

/**
 * @file
 * Database interface code for MySQL database servers using the mysqli client libraries. mysqli is included in PHP 5 by default and allows developers to use the advanced features of MySQL 4.1.x, 5.0.x and beyond.
 */

/* Maintainers of this file should consult
 * http://www.php.net/manual/en/ref.mysqli.php
 */

/**
 * @ingroup database
 * @{
 */

/**
 * Initialise a database connection.
 *
 * Note that mysqli does not support persistent connections.
 */
function db_connect($url) {
  $url = parse_url($url);

  // Allow for non-standard MySQL port.
  if (isset($url['port'])) {
     $url['host'] = $url['host'] .':'. $url['port'];
  }

  $connection = mysqli_init();
  @mysqli_real_connect($connection, $url['host'], $url['user'], $url['pass'], substr($url['path'], 1), NULL, NULL, MYSQLI_CLIENT_FOUND_ROWS);

  if (!$connection) {
    drupal_maintenance_theme();
    drupal_set_title('Unable to connect to database server');
    print theme('maintenance_page', '<p>This either means that the username and password information in your <code>settings.php</code> file is incorrect or we can\'t contact the MySQL database server through the mysqli libraries. This could mean your hosting provider\'s database server is down, or your PHP is not compiled with mysqli.</p>
<p>The MySQL error was: '. theme('placeholder', mysqli_error($connection)) .'.</p>
<p>Currently, the username is '. theme('placeholder', $url['user']) .' and the database server is '. theme('placeholder', $url['host']) .'.</p>
<ul>
  <li>Are you sure you have the correct username and password?</li>
  <li>Are you sure that you have typed the correct hostname?</li>
  <li>Are you sure that the database server is running?</li>
  <li>Are you sure that the mysqli libraries are compiled in your PHP installation? Try using the mysql library instead by editing your <code>settings.php</code> configuration file in Drupal.</li>
</ul>
<p>For more help, see the <a href="http://drupal.org/node/258">Installation and upgrading handbook</a>. If you are unsure what these terms mean you should probably contact your hosting provider.</p>');
    exit;
  }

  if (!mysqli_select_db($connection, substr($url['path'], 1))) {
    drupal_maintenance_theme();
    drupal_set_title('Unable to select database');
    print theme('maintenance_page', '<p>We were able to connect to the MySQL database server (which means your username and password are okay) but not able to select the database.</p>
<p>The MySQL error was: '. theme('placeholder', mysqli_error($connection)) .'.</p>
<p>Currently, the database is '. theme('placeholder', substr($url['path'], 1)) .'. The username is '. theme('placeholder', $url['user']) .' and the database server is '. theme('placeholder', $url['host']) .'.</p>
<ul>
  <li>Are you sure you have the correct database name?</li>
  <li>Are you sure the database exists?</li>
  <li>Are you sure the username has permission to access the database?</li>
</ul>
<p>For more help, see the <a href="http://drupal.org/node/258">Installation and upgrading handbook</a>. If you are unsure what these terms mean you should probably contact your hosting provider.</p>');
    exit;
  }

  return $connection;
}

/**
 * Helper function for db_query().
 */
function _db_query($query, $debug = 0) {
  global $active_db, $queries;

  if (variable_get('dev_query', 0)) {
    list($usec, $sec) = explode(' ', microtime());
    $timer = (float)$usec + (float)$sec;
  }

  $result = mysqli_query($active_db, $query);

  if (variable_get('dev_query', 0)) {
    list($usec, $sec) = explode(' ', microtime());
    $stop = (float)$usec + (float)$sec;
    $diff = $stop - $timer;
    $queries[] = array($query, $diff);
  }

  if ($debug) {
    print '<p>query: '. $query .'<br />error:'. mysqli_error($active_db) .'</p>';
  }

  if (!mysqli_errno($active_db)) {
    return $result;
  }
  else {
    trigger_error(mysqli_error($active_db) ."\nquery: ". htmlspecialchars($query), E_USER_ERROR);
    return FALSE;
  }
}

/**
 * Fetch one result row from the previous query as an object.
 *
 * @param $result
 *   A database query result resource, as returned from db_query().
 * @return
 *   An object representing the next row of the result. The attributes of this
 *   object are the table fields selected by the query.
 */
function db_fetch_object($result) {
  if ($result) {
    return mysqli_fetch_object($result);
  }
}

/**
 * Fetch one result row from the previous query as an array.
 *
 * @param $result
 *   A database query result resource, as returned from db_query().
 * @return
 *   An associative array representing the next row of the result. The keys of
 *   this object are the names of the table fields selected by the query, and
 *   the values are the field values for this result row.
 */
function db_fetch_array($result) {
  if ($result) {
    return mysqli_fetch_array($result, MYSQLI_ASSOC);
  }
}

/**
 * Determine how many result rows were found by the preceding query.
 *
 * @param $result
 *   A database query result resource, as returned from db_query().
 * @return
 *   The number of result rows.
 */
function db_num_rows($result) {
  if ($result) {
    return mysqli_num_rows($result);
  }
}

/**
* Return an individual result field from the previous query.
*
* Only use this function if exactly one field is being selected; otherwise,
* use db_fetch_object() or db_fetch_array().
*
* @param $result
*   A database query result resource, as returned from db_query().
* @param $row
*   The index of the row whose result is needed.
* @return
*   The resulting field.
*/
function db_result($result, $row = 0) {
  if ($result && mysqli_num_rows($result) > $row) {
    $array = mysqli_fetch_array($result, MYSQLI_NUM);
    return $array[0];
  }
}

/**
 * Determine whether the previous query caused an error.
 */
function db_error() {
  global $active_db;
  return mysqli_errno($active_db);
}

/**
 * Return a new unique ID in the given sequence.
 *
 * For compatibility reasons, Drupal does not use auto-numbered fields in its
 * database tables. Instead, this function is used to return a new unique ID
 * of the type requested. If necessary, a new sequence with the given name
 * will be created.
 */
function db_next_id($name) {
  $name = db_prefix_tables($name);
  db_query('LOCK TABLES {sequences} WRITE');
  $id = db_result(db_query("SELECT id FROM {sequences} WHERE name = '%s'", $name)) + 1;
  db_query("REPLACE INTO {sequences} VALUES ('%s', %d)", $name, $id);
  db_query('UNLOCK TABLES');

  return $id;
}

/**
 * Determine the number of rows changed by the preceding query.
 */
function db_affected_rows() {
  global $active_db; /* mysqli connection resource */
  return mysqli_affected_rows($active_db);
}

/**
 * Runs a limited-range query in the active database.
 *
 * Use this as a substitute for db_query() when a subset of the query is to be
 * returned.
 * User-supplied arguments to the query should be passed in as separate parameters
 * so that they can be properly escaped to avoid SQL injection attacks.
 *
 * @param $query
 *   A string containing an SQL query.
 * @param ...
 *   A variable number of arguments which are substituted into the query using
 *   printf() syntax. Instead of a variable number of query arguments, you may
 *   also pass a single array containing the query arguments.
 * @param $from
 *   The first result row to return.
 * @param $count
 *   The maximum number of result rows to return.
 * @return
 *   A database query result resource, or FALSE if the query was not executed
 *   correctly.
 */
function db_query_range($query) {
  $args = func_get_args();
  $count = array_pop($args);
  $from = array_pop($args);

  $query = db_prefix_tables($query);
  if (count($args) > 1) {
    // Check for array (alternative syntax).
    if (is_array($args[1])) {
      $args = array_merge(array($query), $args[1]);
    }
    $args = array_map('db_escape_string', $args);
    $args[0] = $query;
    $query = call_user_func_array('sprintf', $args);
  }
  $query .= ' LIMIT '. $from .', '. $count;
  return _db_query($query);
}

/**
 * Returns a properly formatted Binary Large OBject value.
 *
 * @param $data
 *   Data to encode.
 * @return
 *  Encoded data.
 */
function db_encode_blob($data) {
  return $data;
}

/**
 * Returns text from a Binary Large OBject value.
 *
 * @param $data
 *   Data to decode.
 * @return
 *  Decoded data.
 */
function db_decode_blob($data) {
  return $data;
}

/**
 * Prepare user input for use in a database query, preventing SQL injection attacks.
 */
function db_escape_string($text) {
  return addslashes($text);
}


/**
 * Lock a table.
 */
function db_lock_table($table) {
  db_query('LOCK TABLES {%s} WRITE', $table);
}

/**
 * Unlock all locked tables.
 */
function db_unlock_tables() {
  db_query('UNLOCK TABLES');
}

/**
 * @} End of "ingroup database".
 */
