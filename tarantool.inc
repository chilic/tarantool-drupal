<?php

require_once dirname(__FILE__) . '/tarantool.api.inc';

class TarantoolDrupal implements DrupalCacheInterface {

  /**
   * Name of cache bin.
   * Example: 'cache' or 'cache_page', etc.
   */
  protected $bin;
  protected static $spaces;
  protected $space;

  /**
   * Constructs a new TarantoolDrupal object.
   */
  function __construct($bin) {
    $this->bin = $bin;

    if (!self::$spaces) {
      self::$spaces = variable_get('tarantool_spaces', array('default' => 0));
    }

    $this->space = (isset(self::$spaces[$bin])) ? self::$spaces['bin'] : self::$spaces['default'];
  }

  /**
   * Returns data from the persistent cache.
   *
   * Data may be stored as either plain text or as serialized data. cache_get()
   * will automatically return unserialized objects and arrays.
   *
   * @param $cid
   *   The cache ID of the data to retrieve.
   *
   * @return
   *   The cache or FALSE on failure.
   */
  function get($cid) {
    $result = TarantoolApi::get($this->bin . '-' . $cid, $this->space);

    if ($result) {
      $result = $result[0];
      $cache = new stdClass;
      $cache->cid = $result[0];
      $cache->created = $result[1];
      $cache->serialized = $result[3];
      $cache->data = ($cache->serialized == 1) ? unserialize($result[2]) : $result[2];
      $cache->expire = $result[4];
    }
    else {
      $cache = FALSE;
    }

    //echo $cid . '<br>';

    //return FALSE;
    return $cache;
  }

  /**
   * Returns data from the persistent cache when given an array of cache IDs.
   *
   * @param $cids
   *   An array of cache IDs for the data to retrieve. This is passed by
   *   reference, and will have the IDs successfully returned from cache
   *   removed.
   *
   * @return
   *   An array of the items successfully returned from cache indexed by cid.
   */
  function getMultiple(&$cids) {

    $result = array();

    foreach ($cids as $cid) {
      $cache = $this->get($cid);
      if ($cache) {
        $result[$cid] = $cache;
      }
    }

    return $result;
  }

  /**
   * Stores data in the persistent cache.
   *
   * @param $cid
   *   The cache ID of the data to store.
   * @param $data
   *   The data to store in the cache. Complex data types will be automatically
   *   serialized before insertion.
   *   Strings will be stored as plain text and not serialized.
   * @param $expire
   *   One of the following values:
   *   - CACHE_PERMANENT: Indicates that the item should never be removed unless
   *     explicitly told to using cache_clear_all() with a cache ID.
   *   - CACHE_TEMPORARY: Indicates that the item should be removed at the next
   *     general cache wipe.
   *   - A Unix timestamp: Indicates that the item should be kept at least until
   *     the given time, after which it behaves like CACHE_TEMPORARY.
   */
  function set($cid, $data, $expire = CACHE_PERMANENT) {

    // Create new cache object.
    $cache = new stdClass;
    $cache->cid = $this->bin . '-' . $cid;
    $cache->created = REQUEST_TIME;

    if (is_object($data) || is_array($data)) {
      $cache->data = serialize($data);
      $cache->serialized = 1;
    }
    else {
      $cache->data = $data;
      $cache->serialized = 0;
    }

    if ($expire == CACHE_TEMPORARY) {
      // Convert CACHE_TEMPORARY (-1) into something that will live in memcache
      // until the next flush.
      $cache->expire = REQUEST_TIME + 2591999;
    }
    // Expire time is in seconds if less than 30 days, otherwise is a timestamp.
    else if ($expire != CACHE_PERMANENT && $expire < 2592000) {
      // Expire is expressed in seconds, convert to the proper future timestamp
      // as expected in dmemcache_get().
      $cache->expire = REQUEST_TIME + $expire;
    }
    else {
      $cache->expire = $expire;
    }

    TarantoolApi::set((array) $cache, $this->space);
  }

  /**
   * Expires data from the cache.
   *
   * If called without arguments, expirable entries will be cleared from the
   * cache_page and cache_block bins.
   *
   * @param $cid
   *   If set, the cache ID or an array of cache IDs. Otherwise, all cache
   *   entries that can expire are deleted. The $wildcard argument will be
   *   ignored if set to NULL.
   * @param $wildcard
   *   If TRUE, the $cid argument must contain a string value and cache IDs
   *   starting with $cid are deleted in addition to the exact cache ID
   *   specified by $cid. If $wildcard is TRUE and $cid is '*', the entire
   *   cache is emptied.
   */
  function clear($cid = NULL, $wildcard = FALSE) {
    // TODO: Implement clear() method.
    if ($cid) {
      if (is_array($cid)) {
        foreach ($cid as $c) {
          TarantoolApi::delete($this->bin . '-' . $c, $this->space);
        }
      }
      else {
        TarantoolApi::delete($this->bin . '-' . $cid, $this->space);
      }
    }
  }

  /**
   * Checks if a cache bin is empty.
   *
   * A cache bin is considered empty if it does not contain any valid data for
   * any cache ID.
   *
   * @return
   *   TRUE if the cache bin specified is empty.
   */
  function isEmpty() {
    // TODO: Implement isEmpty() method.
  }
}