const { expect } = require("chai");
const { ethers } = require("hardhat");
const ONE_TLOS = ethers.utils.parseEther("1.0");
const HALF_TLOS = ethers.utils.parseEther("0.5");
const MAX_REQUESTS = 10;

describe("NativeOracleBridge Contract", function () {
    let bridge, owner, oracle, oracle2, user, consumer;
    beforeEach(async () => {
        [owner, oracle, oracle2, user] = await ethers.getSigners();
        let Consumer = await ethers.getContractFactory("NativeOracleConsumer");
        let Bridge = await ethers.getContractFactory("NativeOracleBridge");
        bridge = await Bridge.deploy(ONE_TLOS, MAX_REQUESTS);
        consumer = await Consumer.deploy(bridge.address);
    })
    describe(":: Settings", function () {
        it("Should allow owner to register an Oracle" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            expect(await bridge.oracleBridgeExists(oracle.address)).to.equal(true);
        });
        it("Shouldn't allow addresses other than owner to register an Oracle Bridge" , async function () {
            await expect(bridge.connect(user).registerOracleBridge(oracle2.address, "mygreatoracle2", "rngorcbrdg")).to.be.reverted;
        });
        it("Shouldn't allow addresses other than owner to remove an Oracle Bridge" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            await expect(bridge.connect(user).removeOracleBridge(oracle.address)).to.be.reverted;
        });
        it("Should allow owner to remove an Oracle Bridge" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            await expect(bridge.removeOracleBridge(oracle.address)).to.not.be.reverted;
        });
        it("Should allow owner to set the fee" , async function () {
            await expect(bridge.setFee(HALF_TLOS)).to.not.be.reverted;
            expect(await bridge.fee()).to.equal(HALF_TLOS);
        });
        it("Shouldn't allow addresses other than owner to set the fee" , async function () {
            await expect(bridge.connect(user).setFee(HALF_TLOS)).to.be.reverted;
        });
        it("Should allow owner to set the max requests" , async function () {
            await expect(bridge.setMaxRequests(MAX_REQUESTS / 2)).to.not.be.reverted;
            expect(await bridge.maxRequests()).to.equal(MAX_REQUESTS / 2);
        });
        it("Shouldn't allow addresses other than owner to set the max requests" , async function () {
            await expect(bridge.connect(user).setMaxRequests(MAX_REQUESTS)).to.be.reverted;
        });
    });
    describe(":: Getters", function () {
        it("Should be able to return the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
        it("Should be able to return the max requests" , async function () {
            expect(await bridge.maxRequests()).to.equal(MAX_REQUESTS);
        });
    });
    describe(":: Request", function () {
        it("Should be created with correct parameters" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            await expect( consumer.makeRequest(oracle.address, ["000000018e40"], {"value": ONE_TLOS})).to.not.be.reverted;
        });
        it("Should revert if fee is incorrect" , async function () {
            await expect(consumer.makeRequest(oracle.address, ["000000018e40"], {"value": HALF_TLOS})).to.be.reverted;
        });
        it("Should revert if oracle is not registered" , async function () {
            await expect(consumer.makeRequest(oracle2.address, ["000000018e40"], {"value": ONE_TLOS})).to.be.reverted;
        });
        it("Should not be possible to have more than " + MAX_REQUESTS + " requests" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            for(var i = 0; i < MAX_REQUESTS; i++){
                await expect(consumer.makeRequest(oracle.address, ["000000018e40"], {"value": ONE_TLOS})).to.not.be.reverted;
            }
            await expect(consumer.makeRequest(oracle.address, ["000000018e40"], {"value": ONE_TLOS})).to.be.reverted;
        });
    });
    describe(":: Response", function () {
        it("Shouldn't be able to reply from another address than the Request oracle" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            await expect(consumer.makeRequest(oracle.address, ["arg1", "arg2"], {"value": ONE_TLOS})).to.not.be.reverted;
            await expect(bridge.connect(user).reply(0, consumer.address, ["116"])).to.be.reverted;
        });
        it("Should be able to reply to a Request" , async function () {
            await expect(bridge.registerOracleBridge(oracle.address, "mygreatoracle", "rngorcbrdg")).to.not.be.reverted;
            await expect(consumer.makeRequest(oracle.address, ["arg1", "arg2"], {"value": ONE_TLOS})).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(0, consumer.address, ["116"])).to.not.be.reverted;
        });
    });
});
