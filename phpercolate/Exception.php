<?php

class PercolateException extends Exception {
  private $url;
  private $curl_code;

  public function __construct($message, $code, $url, $curl_code) {
    parent::__construct($message, $code);
    $this->url = $url;
    $this->curl_code = $curl_code;
  }
  
  public function __toString() {
    return __CLASS__ . " $this->code: $this->message at $this->url\n";
  }
  
  public function getUrl() {
    return $this->url;
  }
  
  public function getCurlCode() {
    return $this->curl_code;
  }
}