const OracleBridge = require('./wrappers/OracleBridge');
const { Account } = require('telos_native_test_js');
const config = require('./config.json');

let bridge, owner, user;

before(async () => {
    bridge = new OracleBridge("orc.bridge")
    await bridge.deploy().then(async() => {
        await bridge.init("0x9a469d1e668425907548228EA525A661FF3BFa2B", "1", 'orc.bridge')
    }).catch(async (e) => {
        console.log("  ", e.message[0].toUpperCase() + e.message.substring(1))
    })
})
describe("OracleBridge Contract", function () {
    describe(":: Admin", function () {
        it("Should be able to set the version" , async function () {
            await bridge.setVersion("2");
            expect(await bridge.getVersion()).to.be.equal("2");
        });
        it("Should be able to set the evm_contract" , async function () {
            bridge.connect("tomearhart11");
            try {
                await bridge.setEVMContract("0x9a469d1e668425907548228EA525A661FF3BFa22");
                assert(false, "Was able to set contract")
            } catch (e) { }
            expect(await bridge.getEVMContract()).to.be.equal("0x9a469d1e668425907548228EA525A661FF3BFa22");
            console.log(await bridge.getEVMContract())
        });
        it("Should be able to add an oracle type" , async function () {
            await bridge.addOracleType("rng");
            expect(await bridge.existsOracleType("rng")).to.be.equal(true);
        });
        it("Should be able to add an oracle" , async function () {
            await bridge.addOracle("rng", "myoracle");
            expect(await bridge.existsOracle("myoracle")).to.be.equal(true);
        });
        it("Should be able to remove an oracle" , async function () {
            await bridge.removeOracle("myoracle");
            expect(await bridge.existsOracle("myoracle")).to.be.equal(false);
        });
        it("Should be able to remove an oracle type" , async function () {
            await bridge.removeOracleType("rng");
            expect(await bridge.existsOracleType("rng")).to.be.equal(false);
        });
        it("Should be able to set the admin" , async function () {
        });
    });
    describe(":: Core", function () {
        it("Should be able to make a request" , async function () {
            await bridge.request()
        });
        it("Should be able to reply to EVM" , async function () {

        });
    });
});
after(async () => {
    await bridge.clear();
})
