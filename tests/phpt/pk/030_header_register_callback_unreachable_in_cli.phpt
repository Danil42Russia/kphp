@ok
<?php

echo "Zero";
header_register_callback(function () {
    echo "In CLI is unreachable";
});
echo "One";