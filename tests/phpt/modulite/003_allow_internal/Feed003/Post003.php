<?php

namespace Feed003;

use Utils003\Hidden003;

class Post003 {
    const ONE = 1;
    const TWO = 2;
    const HIDDEN = 3;
    static $ONE = 1;
    static $TWO = 2;
    static $HIDDEN = 3;

    static function demo() {
        \Utils003\Strings003::hidden1();
        Hidden003::demo();
        self::forceHidden();
        plainPublic1();
        plainHidden1();
    }

    static function forceHidden() {
        if(0) Hidden003::demo();
        if(0) globalDemo();
        if(0) \Messages003\Core003\Core003::demo1();
        if(0) plainHidden2();
    }

    static function demoRequireUnexported() {
        require_once __DIR__ . '/../parent/child1/child2/child3/Child3.php';
        \Child3::child3Func();
        $self = new self;
        $self->printHidConst();
    }

    private function printHidConst() {
        echo \Child3::HID, ' ', \Child3::$HID, "\n";
    }
}
