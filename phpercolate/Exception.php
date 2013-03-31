<?php

class PercolateException extends Exception {
  private $url;

  public function __construct($message, $code, $url) {
    parent::__construct($message, $code);
    $this->url = $url;
  }
  
  public function __toString() {
    return __CLASS__ . " $this->code: $this->message at $this->url\n";
  }
  
  public function getUrl() {
    return $this->url;
  }
}