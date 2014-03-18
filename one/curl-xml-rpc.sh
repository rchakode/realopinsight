curl -H "Content-Type: text/xml" -X POST \
  -d '<?xml version="1.0"?>
      <methodCall>
        <methodName>one.hostpool.info</methodName>
        <params>
          <param>
            <value><string>oneadmin:oneadmin</string></value>
          </param>
        </params>
      </methodCall>' http://localhost:2633/RPC2
