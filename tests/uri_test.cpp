#include <gtest/gtest.h>
#include <webpp/utils/uri.h>

using namespace webpp;

TEST(URITests, Creation) {
    ref_uri u("http://example.com/");
    EXPECT_TRUE(u.has_scheme());
    EXPECT_TRUE(u.has_host());
    EXPECT_EQ(u.str(), "http://example.com/");
    EXPECT_EQ(u.host(), "example.com");
    EXPECT_TRUE(is::host(u.str()));
    EXPECT_TRUE(u.has_authority());
    EXPECT_TRUE(u.has_path()) << "the path is '/'";
    EXPECT_FALSE(u.has_port());
    EXPECT_FALSE(u.has_user_info());
    EXPECT_FALSE(u.has_fragment());
    EXPECT_FALSE(u.has_query());
    EXPECT_EQ(u.path_structured().size(), 1);
    EXPECT_EQ(u.scheme(), "http");
    u.clear_scheme();
    EXPECT_FALSE(u.has_scheme());
    EXPECT_EQ(u.scheme(), "");
    EXPECT_TRUE(u.has_authority());
    EXPECT_TRUE(u.has_host());
    EXPECT_TRUE(u.has_path());
    EXPECT_FALSE(u.has_port());
    EXPECT_EQ(u.str(), "//example.com/");
    EXPECT_TRUE(u.is_normalized());

    const_uri ipv4_host("https://192.168.1.1");
    EXPECT_TRUE(is::ipv4(ipv4_host.host()));
    EXPECT_EQ(ipv4_host.scheme(), "https");
    EXPECT_FALSE(ipv4_host.has_path());
    EXPECT_FALSE(ipv4_host.has_query());
    EXPECT_FALSE(ipv4_host.has_port());
    EXPECT_TRUE(ipv4_host.has_authority());
    EXPECT_TRUE(ipv4_host.has_host());
    EXPECT_EQ(ipv4_host.host(), "192.168.1.1");

    ref_uri local_file("file:///home/test/folder/file.txt");
    EXPECT_EQ(local_file.path(), "/home/test/folder/file.txt");
    EXPECT_TRUE(local_file.has_path());
    EXPECT_TRUE(local_file.has_scheme());
    EXPECT_FALSE(local_file.has_authority());
    EXPECT_FALSE(local_file.has_host());
    EXPECT_EQ(local_file.scheme(), "file");
    EXPECT_EQ(local_file.host_decoded(), "");
    auto path = local_file.path_structured_decoded();
    EXPECT_EQ(local_file.path_structured().size(), 5);
    EXPECT_EQ(path.size(), 5);
    EXPECT_EQ(path.at(0), "");
    EXPECT_EQ(path.at(1), "home");
    EXPECT_EQ(path.at(2), "test");
    EXPECT_EQ(path.at(3), "folder");
    EXPECT_EQ(path.at(4), "file.txt");
    EXPECT_TRUE(local_file.is_absolute());
    EXPECT_TRUE(local_file.is_normalized());
    EXPECT_FALSE(local_file.is_relative());
    local_file.clear_path();
    EXPECT_EQ(local_file.str(), "file:///");
}

TEST(URITests, IPv6HostName) {
    uri u;
    std::string uri_str =
        "//[::1]:8080/folder/file.md?name=value&name2=value2#str";
    u = uri_str;
    EXPECT_EQ(u.str(), uri_str);
    EXPECT_FALSE(u.has_scheme());
    EXPECT_TRUE(u.has_host());
    EXPECT_TRUE(u.has_port());
    EXPECT_TRUE(u.has_authority());
    EXPECT_TRUE(u.has_path());
    EXPECT_TRUE(u.has_query());
    EXPECT_TRUE(u.has_fragment());
    EXPECT_EQ(u.fragment(), "str");
    EXPECT_EQ(u.path(), "/folder/file.md");
    EXPECT_EQ(u.host(), "[::1]");
    EXPECT_EQ(u.port_uint16(), 8080);
    EXPECT_EQ(u.port(), "8080");
    EXPECT_TRUE(std::holds_alternative<ipv6>(u.host_structured()));
    u.clear_path();
    EXPECT_EQ(u.str(), "//[::1]:8080/?name=value&name2=value2#str");
}

TEST(URITests, WieredURIs) {
    // some examples from https://rosettacode.org/wiki/URL_parser
    auto _uris = {
        "ftp://ftp.is.co.za/rfc/rfc1808.txt",
        "http://www.ietf.org/rfc/rfc2396.txt",
        "ldap://[2001:db8::7]/c=GB?objectClass?one",
        "mailto:John.Doe@example.com",
        "news:comp.infosystems.www.servers.unix",
        "tel:+1-816-555-1212",
        "telnet://192.0.2.16:80/",
        "urn:oasis:names:specification:docbook:dtd:xml:4.1.2",
        "foo://example.com:8042/over/there?name=ferret#nose",
        "urn:example:animal:ferret:nose",
        "jdbc:mysql://test_user:test@test.com:3306/sakila?profileSQL=true",
        "ftp://ftp.is.co.za/rfc/rfc1808.txt",
        "http://www.ietf.org/rfc/rfc2396.txt#header1",
        "ldap://[2001:db8::7]/c=GB?objectClass=one&objectClass=two",
        "mailto:[email protected]",
        "news:comp.infosystems.www.servers.unix",
        "tel:+1-816-555-1212",
        "telnet://192.0.2.16:80/",
        "urn:oasis:names:specification:docbook:dtd:xml:4.1.2",
        "ssh://test@test.com",
        "https://bob:pass@test.com/place",
        "http://example.com/?a=1&b=2+2&c=3&c=4&d=%65%6e%63%6F%64%65%64"};

    for (auto const& _uri : _uris) {
        EXPECT_TRUE(const_uri(_uri).is_valid());
    }
}

TEST(URITests, URN) {
    auto valid_urns = {
        "urn:isbn:0451450523",
        "urn:uuid:6e8bc430-9c3a-11d9-9669-0800200c9a66",
        "urn:publishing:book",
        "urn:isbn:0451450523",
        "urn:isan:0000-0000-2CEA-0000-1-0000-0000-Y",
        "urn:ISSN:0167-6423",
        "urn:ietf:rfc:2648",
        "urn:mpeg:mpeg7:schema:2001",
        "urn:oid:2.16.840",
        "urn:uuid:6e8bc430-9c3a-11d9-9669-0800200c9a66",
        "urn:nbn:de:bvb:19-146642",
        "urn:lex:eu:council:directive:2010-03-09;2010-19-UE",
        "urn:lsid:zoobank.org:pub:CDC8D258-8F57-41DC-B560-247E17D3DC8C",
        "urn:mpeg:mpeg7:schema:2001urn:isbn:0451450523",
        "urn:sha1:YNCKHTQCWBTRNJIV4WNAE52SJUQCZO5C",
        "urn:uuid:6e8bc430-9c3a-11d9-9669-0800200c9a66"};

    // these are valid urls but they shouldn't be considers part of urn
    auto valid_urls = {"mailto:someone@example.com",
                       "http://foo.com/blah_blah",
                       "http://foo.com/blah_blah/",
                       "http://foo.com/blah_blah_(wikipedia)",
                       "http://foo.com/blah_blah_(wikipedia)_(again)",
                       "http://www.example.com/wpstyle/?p=364",
                       "https://www.example.com/foo/?bar=baz&inga=42&quux",
                       "http://✪df.ws/123",
                       "http://userid:password@example.com:8080",
                       "http://userid:password@example.com:8080/",
                       "http://userid@example.com",
                       "http://userid@example.com/",
                       "http://userid@example.com:8080",
                       "http://userid@example.com:8080/",
                       "http://userid:password@example.com",
                       "http://userid:password@example.com/",
                       "http://142.42.1.1/",
                       "http://142.42.1.1:8080/",
                       "http://➡.ws/䨹",
                       "http://⌘.ws",
                       "http://⌘.ws/",
                       "http://foo.com/blah_(wikipedia)#cite-1",
                       "http://foo.com/blah_(wikipedia)_blah#cite-1",
                       "http://foo.com/unicode_(✪)_in_parens",
                       "http://foo.com/(something)?after=parens",
                       "http://☺.damowmow.com/",
                       "http://code.google.com/events/#&product=browser",
                       "http://j.mp",
                       "ftp://foo.bar/baz",
                       "http://foo.bar/?q=Test%20URL-encoded%20stuff",
                       "http://مثال.إختبار",
                       "http://例子.测试",
                       "http://उदाहरण.परीक्षा",
                       "http://-.~_!$&()*+,;=:%40:80%2f::::::@example.com",
                       "http://1337.net",
                       "http://a.b-c.de",
                       "http://223.255.255.254"};

    const_uri a("urn:example:a123,z456");
    const_uri b = "URN:example:a123,z456";
    const_uri c = "urn:EXAMPLE:a123,z456";

    EXPECT_EQ(a, b);
    EXPECT_EQ(a, c);
    EXPECT_EQ(b, c);
    EXPECT_TRUE(a.is_urn());
    EXPECT_TRUE(b.is_urn());
    EXPECT_TRUE(c.is_urn());
}