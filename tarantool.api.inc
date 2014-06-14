<?php

/**
 * Created by PhpStorm.
 * User: igor
 * Date: 5/29/14
 * Time: 10:06
 */
class TarantoolApi {

  private static $tarantool = FALSE;
  private static $settings = FALSE;

  public static function Connect() {

    // Connect to the taratool.
    if (!self::$tarantool) {

      if (!self::$settings) {
        self::$settings = variable_get('tarantool', array(
            'host' => 'localhost',
            'port' => 33013,
            'second_port' => 33014,
          )
        );
      }

      self::$tarantool = new Tarantool(self::$settings['host'], self::$settings['port'], self::$settings['second_port']);
    }

    return self::$tarantool;
  }


  public static function get($cid, $space = 0, $index = 0) {
    $tnt = self::Connect();

    try {
      $result = $tnt->select($space, $index, $cid); // space,index, key
    } catch (Exception $e) {
      echo "Exception: ", $e->getMessage(), "\n";
    }

    return ($result['count']) ? $result['tuples_list'] : FALSE;
  }

  public static function set($tuple, $space = 0) {
    $tnt = self::Connect();
    try {
      $result = $tnt->insert($space, $tuple); // space, tuple
    } catch (Exception $e) {
      echo "Exception: ", $e->getMessage(), "\n";
    }

    return $result;
  }

  public static function delete($cid, $space = 0) {
    $tnt = self::Connect();
    try {
      $tnt->delete($space, $cid);
    } catch (Exception $e) {
      echo "Exception: ", $e->getMessage(), "\n";
    }
  }
} 