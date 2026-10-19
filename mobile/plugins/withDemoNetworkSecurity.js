const fs = require('node:fs');
const path = require('node:path');
const { withAndroidManifest, withDangerousMod } = require('@expo/config-plugins');

const NETWORK_SECURITY_XML = `<?xml version="1.0" encoding="utf-8"?>
<network-security-config>
    <base-config cleartextTrafficPermitted="false" />
    <domain-config cleartextTrafficPermitted="true">
        <domain includeSubdomains="false">10.0.2.2</domain>
    </domain-config>
</network-security-config>
`;

module.exports = function withDemoNetworkSecurity(config) {
  config = withAndroidManifest(config, (mod) => {
    const application = mod.modResults.manifest.application?.[0];
    if (application) {
      application.$['android:networkSecurityConfig'] = '@xml/network_security_config';
      application.$['android:usesCleartextTraffic'] = 'false';
    }
    return mod;
  });

  return withDangerousMod(config, [
    'android',
    async (mod) => {
      const xmlDirectory = path.join(mod.modRequest.platformProjectRoot, 'app/src/main/res/xml');
      fs.mkdirSync(xmlDirectory, { recursive: true });
      fs.writeFileSync(path.join(xmlDirectory, 'network_security_config.xml'), NETWORK_SECURITY_XML);
      return mod;
    },
  ]);
};
